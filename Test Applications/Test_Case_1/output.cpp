#include <iostream>
#include <thread>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <algorithm>

using namespace std;

const int OBFUSCATION_THREADS = 2;

static queue<pair<int, int>> queues[OBFUSCATION_THREADS];
static mutex mutexes[OBFUSCATION_THREADS];
static condition_variable conditions[OBFUSCATION_THREADS];

static bool stopThreads = false;
static mutex stopMutex;

static atomic<int> g_inFlightTasks{0};
static condition_variable g_allTasksDoneCV;
static mutex g_allTasksDoneMtx;

std::atomic<int>* vec;

std::random_device rd;  
std::mt19937 rng(rd()); 

void initializeArray() {
    vec = new std::atomic<int>[OBFUSCATION_THREADS]; // Dynamically allocate atomic array
    for (int i = 0; i < OBFUSCATION_THREADS; i++) {
        vec[i].store(0);  // Initialize each element to 0
    }
}

void taskFinished() {
    int remaining = --g_inFlightTasks;
    if (remaining == 0) {
        unique_lock<mutex> lock(g_allTasksDoneMtx);
        g_allTasksDoneCV.notify_all();
    }
}

enum {
    FUNC_A,
    FUNC_B,
    FUNC_C,
    FUNC_D,
    FUNC_E
};

enum {
    funcA_lines = 200,
    funcB_lines = 20,
    funcC_lines = 20,
    funcD_lines = 20,
    funcE_lines = 20
};

struct funcD_values {
    int funcD_a;
    int funcD_b;
    int funcD_return;
    bool funcD_done;
};

struct funcE_values {
    int funcE_a;
    int funcE_b;
    int funcE_return;
    bool funcE_done;
};

queue<int> funcD_params_index_pool;
queue<int> funcE_params_index_pool;

vector<funcD_values> funcD_params;
vector<funcE_values> funcE_params;

void execute(int thread_idx);

int getBalancedRandomIndex() {
    // Step 1: Compute the average of the values
    double sum = 0;
    std::vector<int> values(OBFUSCATION_THREADS);
    
    for (int i = 0; i < OBFUSCATION_THREADS; i++) {
        values[i] = vec[i].load();
        sum += values[i];
    }

    double avg = sum / OBFUSCATION_THREADS;
    double threshold = avg * 0.8; // Consider elements below 80% of the average

    std::vector<int> candidateIndices;

    // Step 2: Select indices below the threshold
    for (int i = 0; i < OBFUSCATION_THREADS; i++) {
        if (values[i] <= threshold) {
            candidateIndices.push_back(i);
        }
    }

    // Step 3: If no candidates found, take elements below the median
    if (candidateIndices.empty()) {
        std::vector<int> sortedValues = values;
        std::sort(sortedValues.begin(), sortedValues.end());
        int median = sortedValues[OBFUSCATION_THREADS / 2]; // Approximate median

        for (int i = 0; i < OBFUSCATION_THREADS; i++) {
            if (values[i] <= median) {
                candidateIndices.push_back(i);
            }
        }
    }

    // Step 4: Randomly select from candidates
    std::uniform_int_distribution<int> dist(0, candidateIndices.size() - 1);
    return candidateIndices[dist(rng)];
}

void pushToThread(int funcId, int line_no, int param_index) {
    int thread_idx = getBalancedRandomIndex();
    {
        lock_guard<mutex> lock(mutexes[thread_idx]);
        queues[thread_idx].emplace(funcId, param_index);
        vec[thread_idx].fetch_add(line_no);
        g_inFlightTasks++;
    }
    conditions[thread_idx].notify_one();
}

void funcE(int thread_idx, int param_index) {
    cout << "[funcE] Start on thread " << this_thread::get_id() << endl;
    funcE_params[param_index].funcE_return = 2 * funcE_params[param_index].funcE_a + 2 * funcE_params[param_index].funcE_b;
    funcE_params[param_index].funcE_done = true;
    vec[thread_idx].fetch_sub(funcE_lines);
}

void funcD(int thread_idx, int param_index) {
    cout << "[funcD] Start on thread " << this_thread::get_id() << endl;
    this_thread::sleep_for(chrono::seconds(1));

    int index;
    {
        unique_lock<mutex> lock(mutexes[thread_idx]);
        if (funcE_params_index_pool.empty()) {
            index = funcE_params.size();
            funcE_params.emplace_back();
        }
        else {
            index = funcE_params_index_pool.front();
            funcE_params_index_pool.pop();
        }
        funcE_params[index] = {2, 3, 0, false};
    }
    
    pushToThread(FUNC_E, funcE_lines, index);
    while (!funcE_params[index].funcE_done) {
        if (!queues[thread_idx].empty()) execute(thread_idx);
    }
    
    int res = funcE_params[index].funcE_return;
    funcD_params[param_index].funcD_return = res - funcD_params[param_index].funcD_a + funcD_params[param_index].funcD_b;
    
    cout << "[funcD] End on thread " << res << " " << this_thread::get_id() << endl;
    funcE_params_index_pool.push(index);
    funcD_params[param_index].funcD_done = true;
    vec[thread_idx].fetch_sub(funcD_lines);
}

void funcC(int thread_idx, int param_index) {
    cout << "[funcC] Start on thread " << this_thread::get_id() << endl;
    this_thread::sleep_for(chrono::seconds(1));

    int index;
    {
        unique_lock<mutex> lock(mutexes[thread_idx]);
        if (funcD_params_index_pool.empty()) {
            index = funcD_params.size();
            funcD_params.emplace_back();
        }
        else {
            index = funcD_params_index_pool.front();
            funcD_params_index_pool.pop();
        }
        funcD_params[index] = {4, 5, 0, false};
    }

    pushToThread(FUNC_D, funcD_lines, index);
    while (!funcD_params[index].funcD_done) {
        if (!queues[thread_idx].empty()) execute(thread_idx);
    }
    
    int res = funcD_params[index].funcD_return;
    
    cout << "[funcC] End on thread " << res << " " << this_thread::get_id() << endl;
    funcD_params_index_pool.push(index);
    vec[thread_idx].fetch_sub(funcC_lines);
}

void funcB(int thread_idx, int param_index) {
    cout << "[funcB] Start on thread " << this_thread::get_id() << endl;
    pushToThread(FUNC_C, funcC_lines, -1);
    this_thread::sleep_for(chrono::seconds(2));
    cout << "[funcB] End on thread " << this_thread::get_id() << endl;
    vec[thread_idx].fetch_sub(funcB_lines);
}

void funcA(int thread_idx, int param_index) {
    cout << "[funcA] Start on thread " << this_thread::get_id() << endl;
    pushToThread(FUNC_B, funcB_lines, -1);
    this_thread::sleep_for(chrono::seconds(10));
    cout << "[funcA] End on thread " << this_thread::get_id() << endl;
    vec[thread_idx].fetch_sub(funcA_lines);
}

void execute(int thread_idx) {    
    if (queues[thread_idx].empty()) return;
    pair<int, int> func_info = queues[thread_idx].front();
    queues[thread_idx].pop();
    
    switch (func_info.first) {
        case FUNC_A: funcA(thread_idx, func_info.second); break;
        case FUNC_B: funcB(thread_idx, func_info.second); break;
        case FUNC_C: funcC(thread_idx, func_info.second); break;
        case FUNC_D: funcD(thread_idx, func_info.second); break;
        case FUNC_E: funcE(thread_idx, func_info.second); break;
        default: cout << "Unknown funcId " << func_info.first << endl; break;
    }

    taskFinished();
}

void threadFunction(int thread_idx) {
    while (true) {
        {
            unique_lock<mutex> lock(mutexes[thread_idx]);
            conditions[thread_idx].wait(lock, [&]{ return !queues[thread_idx].empty() || stopThreads; });
        }
        
        if (stopThreads && queues[thread_idx].empty()) break;
        execute(thread_idx);
    }
}

int main() {
    thread threads[OBFUSCATION_THREADS];
    initializeArray();

    for (int i = 0; i < OBFUSCATION_THREADS; i++) {
        threads[i] = thread(threadFunction, i);
    }

    pushToThread(FUNC_A, funcA_lines, -1);

    unique_lock<mutex> lock(g_allTasksDoneMtx);
    g_allTasksDoneCV.wait(lock, []{ return g_inFlightTasks.load() == 0; });
    
    {
        lock_guard<mutex> stopLock(stopMutex);
        stopThreads = true;
    }
    
    for (int i = 0; i < OBFUSCATION_THREADS; i++) {
        conditions[i].notify_all();
        threads[i].join();
    }
    
    return 0;
}
