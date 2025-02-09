#ifndef OBFUSCATOR_H
#define OBFUSCATOR_H

#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <random>
#include <thread>

using namespace std;

constexpr int OBFUSCATION_THREADS = 2; // Declared as extern since it's defined in obfuscator.cpp

// Enums for function identifiers
enum FunctionID {
    FUNC_A,
    FUNC_B,
    FUNC_C,
    FUNC_D,
    FUNC_E
};

// Enums for function line counts
enum FunctionLines {
    funcA_lines = 200,
    funcB_lines = 20,
    funcC_lines = 20,
    funcD_lines = 20,
    funcE_lines = 20
};

// Structs for function-specific values
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

// Function parameter pools and storage
extern queue<int> funcD_params_index_pool;
extern queue<int> funcE_params_index_pool;

extern vector<funcD_values> funcD_params;
extern vector<funcE_values> funcE_params;

// Queues for task distribution
extern thread threads[OBFUSCATION_THREADS];
extern queue<pair<int, int>> queues[OBFUSCATION_THREADS];
extern mutex mutexes[OBFUSCATION_THREADS];
extern condition_variable conditions[OBFUSCATION_THREADS];

// Thread control flags
extern bool stopThreads;
extern mutex stopMutex;

// Global task tracking
extern atomic<int> g_inFlightTasks;
extern condition_variable g_allTasksDoneCV;
extern mutex g_allTasksDoneMtx;

// Atomic vector for load balancing
extern std::atomic<int>* vec;

// Function prototypes
void initializeArray();
void exit();
void taskFinished();
int getBalancedRandomIndex();
void pushToThread(int funcId, int line_no, int param_index);
void execute(int thread_idx);
void threadFunction(int thread_idx);

// Function declarations
void funcA(int thread_idx, int param_index);
void funcB(int thread_idx, int param_index);
void funcC(int thread_idx, int param_index);
void funcD(int thread_idx, int param_index);
void funcE(int thread_idx, int param_index);

#endif // OBFUSCATOR_H