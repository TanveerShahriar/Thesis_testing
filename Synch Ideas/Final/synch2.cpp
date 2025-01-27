#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include <vector>
#include <stack>
#include <atomic>

using namespace std;

// ============================================================================
//                     Global / Shared Structures
// ============================================================================


const int OBFUSCATION_THREADS = 2;

static vector<stack<int>> stacks;

static vector<pthread_mutex_t> mutexes;
static vector<pthread_cond_t> conditions;

static bool stopThreads = false;
static pthread_mutex_t stopMutex = PTHREAD_MUTEX_INITIALIZER;

static atomic<int> g_inFlightTasks{0};
static pthread_cond_t  g_allTasksDoneCV  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t g_allTasksDoneMtx = PTHREAD_MUTEX_INITIALIZER;

void taskFinished() {
    int remaining = --g_inFlightTasks; // Decrement the global counter
    if (remaining == 0) {
        // If no tasks left, wake anyone waiting on g_allTasksDoneCV
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

struct funcA_values
{

};

struct funcB_values
{

};

struct funcC_values
{

};

struct funcD_values
{
    int funcD_a;
    int funcD_b;
    int funcD_return;

    bool funcD_done = false;
};

struct funcE_values
{
    int funcE_a;
    int funcE_b;
    int funcE_return;

    bool funcE_done = false;
};

stack<funcA_values> funcA_value_stack;
stack<funcB_values> funcB_value_stack;
stack<funcC_values> funcC_value_stack;
stack<funcD_values> funcD_value_stack;
stack<funcE_values> funcE_value_stack;

void execute(int thread_idx);

// ============================================================================
//               Helpers to "push a function" onto a thread
// ============================================================================

void pushToThread(int funcId, int thread_idx) {
    pthread_mutex_lock(&mutexes[thread_idx]);
    stacks[thread_idx].push(funcId);
    g_inFlightTasks++;
    pthread_cond_signal(&conditions[thread_idx]);
    pthread_mutex_unlock(&mutexes[thread_idx]);
}


// ============================================================================
//                       Actual "Functions"
// ============================================================================

void funcE(){
    cout << "[funcE] Start on pthread " << pthread_self() << endl;

    funcE_value_stack.top().funcE_return = 2 * funcE_value_stack.top().funcE_a + 2 * funcE_value_stack.top().funcE_b;
    funcE_value_stack.top().funcE_done = true;
}

void funcD(){
    cout << "[funcD] Start on pthread " << pthread_self() << endl;
    sleep(1);

    funcE_values funcE_val_1;
    funcE_val_1.funcE_a = 2;
    funcE_val_1.funcE_b = 3;

    funcE_value_stack.push(funcE_val_1);

    pushToThread(FUNC_E, 0);

    while (!funcE_value_stack.top().funcE_done)
    {

    }
    
    int res = funcE_value_stack.top().funcE_return;
    funcE_value_stack.pop();

    funcD_value_stack.top().funcD_return = res - funcD_value_stack.top().funcD_a + funcD_value_stack.top().funcD_b;

    cout << "End D " << res << " " << pthread_self() << endl;

    funcD_value_stack.top().funcD_done = true;
}

void funcC(){
    cout << "[funcC] Start on pthread " << pthread_self() << endl;

    sleep(1);

    funcD_values funcD_val_1;
    funcD_val_1.funcD_a = 4;
    funcD_val_1.funcD_b = 5;

    funcD_value_stack.push(funcD_val_1);
    
    int temp;
    if (stacks[0].empty()) temp = -1;
    else temp = stacks[0].top();
    pushToThread(FUNC_D, 1);

    while (!funcD_value_stack.top().funcD_done)
    {
        int t;
        if (stacks[0].empty()) t = -1;
        else t = stacks[0].top();
        if (temp != t){
            execute(0);
        }
    }
    
    int res = funcD_value_stack.top().funcD_return;
    funcD_value_stack.pop();

    cout << "[funcC] End on pthread " << res << " " << pthread_self() << endl;
}

void funcB(){
    cout << "[funcB] Start on pthread " << pthread_self() << endl;

    sleep(2);
    pushToThread(FUNC_C, 0);

    cout << "[funcB] End on pthread " << pthread_self() << endl;
}

void funcA(){
    cout << "[funcA] Start on pthread " << pthread_self() << endl;

    sleep(1);
    pushToThread(FUNC_B, 1);

    sleep(2);

    cout << "[funcA] End on pthread " << pthread_self() << endl;
}


// ============================================================================
//                        Thread Main Loops
// ============================================================================

void execute(int thread_idx){
    int funcId = stacks[thread_idx].top();
    stacks[thread_idx].pop();
    pthread_mutex_unlock(&mutexes[thread_idx]);

    switch (funcId) {
        case FUNC_A:  funcA();  break;
        case FUNC_B:  funcB();  break;
        case FUNC_C:  funcC();  break;
        case FUNC_D:  funcD();  break;
        case FUNC_E:  funcE();  break;
        default:
            cout << "Thread1: Unknown funcId " << funcId << endl;
            break;
    }

    taskFinished();
}

void* threadFunction(void* arg){
    int thread_idx = *(int*)arg;
    // !stopThreads || !stacks[thread_idx].empty()
    while (true) {
        while (stacks[thread_idx].empty() && !stopThreads)
        {
            pthread_cond_wait(&conditions[thread_idx], &mutexes[thread_idx]);
        }

        if (stopThreads && stacks[thread_idx].empty()) {
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
        stacks.push_back(stack<int>()); 

        pthread_mutex_t mutex;
        pthread_mutex_init(&mutex, nullptr);
        mutexes.push_back(mutex);

        pthread_cond_t cond;
        pthread_cond_init(&cond, nullptr);
        conditions.push_back(cond);

        int* thread_idx = new int(i);
        if (pthread_create(&threads[i], nullptr, threadFunction, thread_idx) != 0) {
            cerr << "Error creating thread " << i + 1 << endl;
            return 1; // Exit on error
        }
    }





    pushToThread(FUNC_A, 0);





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


    for(int i = 0; i < OBFUSCATION_THREADS; i++){
        pthread_cond_broadcast(&conditions[i]);
        if (pthread_join(threads[i], nullptr) != 0) {
            cerr << "Error joining thread " << i + 1 << endl;
            return 2; // Exit on error
        }
        pthread_mutex_destroy(&mutexes[i]);
        pthread_cond_destroy(&conditions[i]);
    }
    return 0;
}