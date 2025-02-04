#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include <queue>
#include <atomic>

using namespace std;

// ============================================================================
//                     Global / Shared Structures
// ============================================================================

const int OBFUSCATION_THREADS = 2;

static queue<int> queues[OBFUSCATION_THREADS];
static pthread_mutex_t mutexes[OBFUSCATION_THREADS];
static pthread_cond_t conditions[OBFUSCATION_THREADS];

static bool stopThreads = false;
static pthread_mutex_t stopMutex = PTHREAD_MUTEX_INITIALIZER;

static atomic<int> g_inFlightTasks{0};
static pthread_cond_t g_allTasksDoneCV = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t g_allTasksDoneMtx = PTHREAD_MUTEX_INITIALIZER;

void taskFinished() {
    int remaining = --g_inFlightTasks;
    if (remaining == 0) {
        pthread_mutex_lock(&g_allTasksDoneMtx);
        pthread_cond_broadcast(&g_allTasksDoneCV);
        pthread_mutex_unlock(&g_allTasksDoneMtx);
    }
}

enum {
    FUNC_A,
    FUNC_B,
    FUNC_C,
    FUNC_D,
    FUNC_E
};

struct funcD_values {
    int funcD_a;
    int funcD_b;
    int funcD_return;
    bool funcD_done = false;
};

struct funcE_values {
    int funcE_a;
    int funcE_b;
    int funcE_return;
    bool funcE_done = false;
};

queue<funcD_values> funcD_value_queue;
queue<funcE_values> funcE_value_queue;

void execute(int thread_idx);

// ============================================================================
//               Helpers to "push a function" onto a thread
// ============================================================================

void pushToThread(int funcId, int thread_idx) {
    pthread_mutex_lock(&mutexes[thread_idx]);
    queues[thread_idx].push(funcId);
    g_inFlightTasks++;
    pthread_cond_signal(&conditions[thread_idx]);
    pthread_mutex_unlock(&mutexes[thread_idx]);
}

// ============================================================================
//                       Actual "Functions"
// ============================================================================

void funcE(int thread_idx){
    cout << "[funcE] Start on pthread " << pthread_self() << endl;
    funcE_value_queue.front().funcE_return = 2 * funcE_value_queue.front().funcE_a + 2 * funcE_value_queue.front().funcE_b;
    funcE_value_queue.front().funcE_done = true;
}

void funcD(int thread_idx){
    cout << "[funcD] Start on pthread " << pthread_self() << endl;
    sleep(1);

    funcE_values funcE_val_1 {2, 3, 0, false};
    funcE_value_queue.push(funcE_val_1);

    pushToThread(FUNC_E, 0);
    while (!funcE_value_queue.front().funcE_done) {
        if (!queues[1].empty()) execute(thread_idx);
    }

    int res = funcE_value_queue.front().funcE_return;
    funcE_value_queue.pop();
    funcD_value_queue.front().funcD_return = res - funcD_value_queue.front().funcD_a + funcD_value_queue.front().funcD_b;

    cout << "End D " << res << " " << pthread_self() << endl;
    funcD_value_queue.front().funcD_done = true;
}

void funcC(int thread_idx){
    cout << "[funcC] Start on pthread " << pthread_self() << endl;
    sleep(1);

    funcD_values funcD_val_1 {4, 5, 0, false};
    funcD_value_queue.push(funcD_val_1);

    pushToThread(FUNC_D, 1);
    while (!funcD_value_queue.front().funcD_done) {
        if (!queues[0].empty()) execute(thread_idx);
    }

    int res = funcD_value_queue.front().funcD_return;
    funcD_value_queue.pop();

    cout << "[funcC] End on pthread " << res << " " << pthread_self() << endl;
}

void funcB(int thread_idx){
    cout << "[funcB] Start on pthread " << pthread_self() << endl;

    pushToThread(FUNC_C, 0);
    
    sleep(2);
    cout << "[funcB] End on pthread " << pthread_self() << endl;
}

void funcA(int thread_idx){
    cout << "[funcA] Start on pthread " << pthread_self() << endl;

    pushToThread(FUNC_B, 1);
    
    sleep(1);
    cout << "[funcA] End on pthread " << pthread_self() << endl;
}

// ============================================================================
//                        Thread Main Loops
// ============================================================================

void execute(int thread_idx){
    int funcId = queues[thread_idx].front();
    queues[thread_idx].pop();
    pthread_mutex_unlock(&mutexes[thread_idx]);

    switch (funcId) {
        case FUNC_A: funcA(thread_idx); break;
        case FUNC_B: funcB(thread_idx); break;
        case FUNC_C: funcC(thread_idx); break;
        case FUNC_D: funcD(thread_idx); break;
        case FUNC_E: funcE(thread_idx); break;
        default: cout << "Unknown funcId " << funcId << endl; break;
    }

    taskFinished();
}

void* threadFunction(void* arg){
    int thread_idx = *(int*)arg;
    while (true) {
        while (queues[thread_idx].empty() && !stopThreads) {
            pthread_cond_wait(&conditions[thread_idx], &mutexes[thread_idx]);
        }

        if (stopThreads && queues[thread_idx].empty()) {
            pthread_mutex_unlock(&mutexes[thread_idx]);
            break;
        }

        execute(thread_idx);
    }
    return nullptr;
}

int main(){
    pthread_t threads[OBFUSCATION_THREADS];

    for(int i = 0; i < OBFUSCATION_THREADS; i++){
        pthread_mutex_init(&mutexes[i], nullptr);
        pthread_cond_init(&conditions[i], nullptr);

        int* thread_idx = new int(i);
        if (pthread_create(&threads[i], nullptr, threadFunction, thread_idx) != 0) {
            cerr << "Error creating thread " << i + 1 << endl;
            return 1;
        }
    }

    pushToThread(FUNC_A, 0);
    
    pthread_mutex_lock(&g_allTasksDoneMtx);
    while (g_inFlightTasks.load() != 0) {
        pthread_cond_wait(&g_allTasksDoneCV, &g_allTasksDoneMtx);
    }
    pthread_mutex_unlock(&g_allTasksDoneMtx);
    
    pthread_mutex_lock(&stopMutex);
    stopThreads = true;
    pthread_mutex_unlock(&stopMutex);
    
    for(int i = 0; i < OBFUSCATION_THREADS; i++){
        pthread_cond_broadcast(&conditions[i]);
        if (pthread_join(threads[i], nullptr) != 0) {
            cerr << "Error joining thread " << i + 1 << endl;
            return 2;
        }
        pthread_mutex_destroy(&mutexes[i]);
        pthread_cond_destroy(&conditions[i]);
    }
    return 0;
}