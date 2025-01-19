#include <iostream>
#include <pthread.h>
#include <unistd.h>

// --------------------------------------------------
// Global synchronization objects
// --------------------------------------------------
pthread_t threadA, threadB;

// A mutex to protect shared data
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

// Condition variables:
//   cond_cRequest:  B -> A "Please run C with args"
//   cond_cDone:     A -> B "C is finished"
pthread_cond_t cond_cRequest = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_cDone    = PTHREAD_COND_INITIALIZER;

// Shared flags/data
bool cRequested = false;
bool cDone      = false;
int  cArg1      = 0;  // argument 1 for funcC
int  cArg2      = 0;  // argument 2 for funcC
int  cResult    = 0;  // result from funcC

// --------------------------------------------------
// Function C - must run in A's thread
// --------------------------------------------------
int funcC(int x, int y) {
    std::cout << "S Func C (thread " << pthread_self() << "), x=" << x << ", y=" << y << "\n";
    sleep(2); // Simulate some work
    int sum = x + y;  // compute sum
    std::cout << "E Func C (thread " << pthread_self() 
              << "), sum=" << sum << "\n";
    return sum;
}

// --------------------------------------------------
// B "calls" C but physically it is run in A's thread
// --------------------------------------------------
int requestC(int x, int y)
{
    // Lock the mutex before using shared data
    pthread_mutex_lock(&m);

    // Prepare arguments for C
    cArg1 = x;
    cArg2 = y;

    // Tell A that we want C to be run
    cRequested = true;
    pthread_cond_signal(&cond_cRequest);

    // Now wait until A signals that C is done
    while (!cDone) {
        pthread_cond_wait(&cond_cDone, &m);
    }
    // Reset for cleanliness if you want to do multiple calls
    cDone = false;

    // Grab the result of C
    int result = cResult;

    pthread_mutex_unlock(&m);

    return result;
}

// --------------------------------------------------
// B in its own thread
// --------------------------------------------------
void funcB() {
    std::cout << "S Func B (thread " << pthread_self() << ")\n";
    sleep(2); // Simulate some work

    // B "calls" C, but physically it is run in A's thread
    // Suppose we want to compute sum of (10, 20)
    std::cout << "B requesting C(10, 20) ...\n";
    int w = 30;
    int x = 40;
    int retFromC = requestC(w, x);
    std::cout << "B got result from C: " << retFromC
              << " (thread " << pthread_self() << ")\n";

    std::cout << "E Func B (thread " << pthread_self() << ")\n";
}

void* threadFunctionB(void* /*arg*/) {
    funcB();
    return nullptr;
}

// --------------------------------------------------
// A in its own thread
// --------------------------------------------------
void funcA() {
    std::cout << "S Func A (thread " << pthread_self() << ")\n";
    sleep(2); // Simulate some work

    // Create B
    pthread_create(&threadB, nullptr, threadFunctionB, nullptr);

    // A might do other stuff, but eventually it must
    // wait for B's request to run C:
    pthread_mutex_lock(&m);
    while (!cRequested) {
        pthread_cond_wait(&cond_cRequest, &m);
    }
    // Clear the request flag
    cRequested = false;

    // Now run C in A's thread, using the arguments set by B
    cResult = funcC(cArg1, cArg2);

    // Signal B that C is done
    cDone = true;
    pthread_cond_signal(&cond_cDone);
    pthread_mutex_unlock(&m);

    // Wait for B to finish
    pthread_join(threadB, nullptr);

    std::cout << "E Func A (thread " << pthread_self() << ")\n";
}

void* threadFunctionA(void* /*arg*/) {
    funcA();
    return nullptr;
}

// --------------------------------------------------
// Main
// --------------------------------------------------
int main() {
    pthread_create(&threadA, nullptr, threadFunctionA, nullptr);
    pthread_join(threadA, nullptr);
    return 0;
}
