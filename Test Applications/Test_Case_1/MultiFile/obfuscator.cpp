#include <algorithm>
#include "obfuscator.hpp"

thread threads[OBFUSCATION_THREADS];

// Queues for each thread
queue<pair<int, int>> queues[OBFUSCATION_THREADS];
mutex mutexes[OBFUSCATION_THREADS];
condition_variable conditions[OBFUSCATION_THREADS];

bool stopThreads = false;
mutex stopMutex;

atomic<int> g_inFlightTasks{0};
condition_variable g_allTasksDoneCV;
mutex g_allTasksDoneMtx;

queue<int> funcD_params_index_pool;
queue<int> funcE_params_index_pool;

vector<funcD_values> funcD_params;
vector<funcE_values> funcE_params;

std::atomic<int>* vec;

std::random_device rd;
std::mt19937 rng(rd());

void initialize() {
    vec = new std::atomic<int>[OBFUSCATION_THREADS];
    for (int i = 0; i < OBFUSCATION_THREADS; i++) {
        vec[i].store(0);
    }

    for (int i = 0; i < OBFUSCATION_THREADS; i++) {
        threads[i] = thread(threadFunction, i);
    }
}

void exit(){
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
}

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

void taskFinished() {
    int remaining = --g_inFlightTasks;
    if (remaining == 0) {
        unique_lock<mutex> lock(g_allTasksDoneMtx);
        g_allTasksDoneCV.notify_all();
    }
}

void execute(int thread_idx) {
    if (queues[thread_idx].empty()) return;

    pair<int, int> func_info;
    {
        lock_guard<mutex> lock(mutexes[thread_idx]);
        func_info = queues[thread_idx].front();
        queues[thread_idx].pop();
    }

    switch (func_info.first) {
        case FUNC_A: funcA(thread_idx, func_info.second); break;
        case FUNC_B: funcB(thread_idx, func_info.second); break;
        case FUNC_C: funcC(thread_idx, func_info.second); break;
        case FUNC_D: funcD(thread_idx, func_info.second); break;
        case FUNC_E: funcE(thread_idx, func_info.second); break;
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
