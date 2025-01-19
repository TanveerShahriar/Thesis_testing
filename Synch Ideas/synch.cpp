#include <iostream>
#include <vector>
#include <atomic>
#include <pthread.h>
#include <unistd.h>

// For convenience
using namespace std;

// ============================================================================
//                     Global / Shared Structures
// ============================================================================

// Two "stacks" (vectors) for tasks, one per thread.
static vector<int> stack1;  // Thread 1's stack
static vector<int> stack2;  // Thread 2's stack

// Mutexes to protect each stack:
static pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

// Condition variables to signal each thread when work appears:
static pthread_cond_t cv1 = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cv2 = PTHREAD_COND_INITIALIZER;

// A flag to stop the threads (once no more tasks remain):
static bool stopThreads = false;
// Mutex to protect stopThreads (not strictly necessary but good practice):
static pthread_mutex_t stopMutex = PTHREAD_MUTEX_INITIALIZER;

// ----------------------------------------------------------------------------
//  In-Flight Task Counter
// ----------------------------------------------------------------------------
//
// We increment this counter each time we enqueue a new function onto a thread’s 
// stack, and decrement it right after we finish running that function.  
// When this count goes to 0, we know there are no pending or running tasks.
//
// We'll also need a condition variable to let the main thread wait until
// all tasks are done (g_inFlightTasks == 0).

static atomic<int> g_inFlightTasks{0};

// Condition + mutex for "all tasks done"
static pthread_cond_t  g_allTasksDoneCV  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t g_allTasksDoneMtx = PTHREAD_MUTEX_INITIALIZER;

// Helper to notify that a task is finished
void taskFinished() {
    int remaining = --g_inFlightTasks; // Decrement the global counter
    if (remaining == 0) {
        // If no tasks left, wake anyone waiting on g_allTasksDoneCV
        pthread_mutex_lock(&g_allTasksDoneMtx);
        pthread_cond_broadcast(&g_allTasksDoneCV);
        pthread_mutex_unlock(&g_allTasksDoneMtx);
    }
}

// ============================================================================
//                       Function IDs and Globals
// ============================================================================

// Let's define an enum to map function IDs to actual functions:
enum {
    FUNC_A  = 1,
    FUNC_B  = 2,
    FUNC_C  = 3,
    FUNC_D1 = 4,
    FUNC_D2 = 5,
    FUNC_E  = 6
};

// Example of some shared “function” data, just to illustrate passing data around
static int funcE_a, funcE_b, funcE_res;
static int funcD_a, funcD_b, funcD_res;

// ============================================================================
//               Helpers to "push a function" onto a thread
// ============================================================================

void pushToThread1(int funcId) {
    pthread_mutex_lock(&m1);
    stack1.push_back(funcId);
    g_inFlightTasks++;   // A new task is added
    pthread_cond_signal(&cv1);
    pthread_mutex_unlock(&m1);
}

void pushToThread2(int funcId) {
    pthread_mutex_lock(&m2);
    stack2.push_back(funcId);
    g_inFlightTasks++;   // A new task is added
    pthread_cond_signal(&cv2);
    pthread_mutex_unlock(&m2);
}

// ============================================================================
//                       Actual "Functions"
// ============================================================================
//
// Each function can do work and optionally push more function IDs onto
// either stack, simulating a "call" from one thread to another.

// funcA: Just an example that pushes funcB onto thread 2
void funcA() {
    cout << "[funcA] Start on pthread " << pthread_self() << endl;

    // Suppose funcA wants to "call" funcB on thread 2
    pushToThread2(FUNC_B);
    sleep(1);

    cout << "[funcA] End on pthread " << pthread_self() << endl;
}

// funcB: "calls" funcC on thread 1
void funcB() {
    cout << "[funcB] Start on pthread " << pthread_self() << endl;

    // Sleep to demonstrate concurrency (not required logically)
    sleep(1);

    // Suppose funcB wants to call funcC on thread 1
    pushToThread1(FUNC_C);

    cout << "[funcB] End on pthread " << pthread_self() << endl;
}

// funcC: sets up data for D, then pushes FUNC_D1 onto thread 2
void funcC() {
    cout << "[funcC] Start on pthread " << pthread_self() << endl;

    funcD_a = 4;
    funcD_b = 5;

    // "Call" funcD_1 on thread 2
    pushToThread2(FUNC_D1);

    cout << "[funcC] End on pthread " << pthread_self() << endl;
}

// funcD_1: sets up data for E, then pushes FUNC_E onto thread 1
void funcD_1() {
    cout << "[funcD_1] Start on pthread " << pthread_self() << endl;

    funcE_a = 2;
    funcE_b = 3;

    // "Call" funcE on thread 1
    pushToThread1(FUNC_E);
}

// funcD_2: finishes using funcE's result
void funcD_2() {
    cout << "[funcD_2] Start on pthread " << pthread_self() << endl;

    int res = funcE_res;
    cout << "[funcD_2] funcE_res = " << res << endl;
    funcD_res = res - funcD_a + funcD_b;
    cout << "[funcD_2] funcD_res = " << funcD_res << endl;
}

// funcE: calculates something, then pushes FUNC_D2 onto thread 2
void funcE() {
    cout << "[funcE] Start on pthread " << pthread_self() << endl;
    funcE_res = 2 * funcE_a + 2 * funcE_b;
    // "Call" funcD_2 on thread 2
    pushToThread2(FUNC_D2);
}

// ============================================================================
//                        Thread Main Loops
// ============================================================================

void* thread1_func(void*) {
    while (true) {
        pthread_mutex_lock(&m1);
        // Wait until we have something on stack1 OR stopThreads is set
        while (stack1.empty() && !stopThreads) {
            pthread_cond_wait(&cv1, &m1);
        }

        // If stop is requested and stack is empty, break
        if (stopThreads && stack1.empty()) {
            pthread_mutex_unlock(&m1);
            break;
        }

        // We have a task
        int funcId = stack1.back();
        stack1.pop_back();
        pthread_mutex_unlock(&m1);

        // Dispatch to the correct function
        switch (funcId) {
            case FUNC_A:  funcA();  break;
            case FUNC_C:  funcC();  break;
            case FUNC_E:  funcE();  break;
            default:
                cout << "Thread1: Unknown funcId " << funcId << endl;
                break;
        }

        // Mark this task as finished
        taskFinished();
    }

    cout << "[Thread1] Exiting.\n";
    return nullptr;
}

void* thread2_func(void*) {
    while (true) {
        pthread_mutex_lock(&m2);
        // Wait until we have something on stack2 OR stopThreads is set
        while (stack2.empty() && !stopThreads) {
            pthread_cond_wait(&cv2, &m2);
        }

        // If stop is requested and stack is empty, break
        if (stopThreads && stack2.empty()) {
            pthread_mutex_unlock(&m2);
            break;
        }

        // We have a task
        int funcId = stack2.back();
        stack2.pop_back();
        pthread_mutex_unlock(&m2);

        // Dispatch
        switch (funcId) {
            case FUNC_B:  funcB();   break;
            case FUNC_D1: funcD_1(); break;
            case FUNC_D2: funcD_2(); break;
            default:
                cout << "Thread2: Unknown funcId " << funcId << endl;
                break;
        }

        // Mark this task as finished
        taskFinished();
    }

    cout << "[Thread2] Exiting.\n";
    return nullptr;
}

// ============================================================================
//                             main()
// ============================================================================
int main() {
    // Create the two worker threads
    pthread_t t1, t2;
    pthread_create(&t1, nullptr, thread1_func, nullptr);
    pthread_create(&t2, nullptr, thread2_func, nullptr);

    // Start something: enqueue FUNC_A onto Thread1
    // (This increments g_inFlightTasks)
    pushToThread1(FUNC_A);

    // ------------------------------------------------------------------------
    // Instead of sleeping, we can wait until all tasks are done.
    // If you know you'll push more tasks from main in a real app, you'd do that
    // here. For this example, let's just wait until everything finishes.

    // Wait for in-flight tasks to become 0
    pthread_mutex_lock(&g_allTasksDoneMtx);
    while (g_inFlightTasks.load() != 0) {
        pthread_cond_wait(&g_allTasksDoneCV, &g_allTasksDoneMtx);
    }
    pthread_mutex_unlock(&g_allTasksDoneMtx);

    // Now we know no tasks remain, so let's request both threads to stop.
    pthread_mutex_lock(&stopMutex);
    stopThreads = true;
    pthread_mutex_unlock(&stopMutex);

    // Broadcast to both threads in case they're waiting with empty stacks
    pthread_cond_broadcast(&cv1);
    pthread_cond_broadcast(&cv2);

    // Join them
    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    cout << "[main] All threads joined.\n";
    cout << "[main] Final: funcD_res = " << funcD_res 
         << ", funcE_res = " << funcE_res << endl;

    return 0;
}