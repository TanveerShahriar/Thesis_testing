#include <iostream>
#include <pthread.h>
#include <unistd.h>

// ------------------------------------------------
// Shared synchronization
// ------------------------------------------------
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;

// 'phase' goes 0..6:
//   0 => A (thread A)
//   1 => B (thread B)
//   2 => E (thread A)
//   3 => D (thread B)
//   4 => C (thread A)
//   5 => B2 (thread B)
//   6 => done
int phase = 0;

// ------------------------------------------------
// Shared variables for passing results
// ------------------------------------------------
// A produces aResult
int aResult = 0;

// B produces bResult, then B2 produces b2Result
int bResult  = 0;
int b2Result = 0;

// E produces eResult
int eResult = 0;

// D produces dResult
int dResult = 0;

// C produces cResult
int cResult = 0;

// ------------------------------------------------
// Forward declarations of the "functions"
// ------------------------------------------------
void funcA();   // Runs in Thread A
void funcB();   // Runs in Thread B
void funcE();   // Runs in Thread A
void funcD();   // Runs in Thread B
void funcC();   // Runs in Thread A
void funcB2();  // Runs again in Thread B

// ------------------------------------------------
// Thread A: handles phases 0, 2, 4
// ------------------------------------------------
void* threadA_func(void* /*unused*/) {
    pthread_mutex_lock(&m);

    while (true) {
        if (phase >= 6) {
            // phase=6 => we are done
            pthread_mutex_unlock(&m);
            break;
        }

        if (phase == 0) {
            // A
            pthread_mutex_unlock(&m);
            funcA();
            pthread_mutex_lock(&m);

            phase = 1;
            pthread_cond_broadcast(&cond);
        }
        else if (phase == 2) {
            // E
            pthread_mutex_unlock(&m);
            funcE();
            pthread_mutex_lock(&m);

            phase = 3;
            pthread_cond_broadcast(&cond);
        }
        else if (phase == 4) {
            // C
            pthread_mutex_unlock(&m);
            funcC();
            pthread_mutex_lock(&m);

            phase = 5;
            pthread_cond_broadcast(&cond);
        }
        else {
            // Not Thread A's turn, wait
            pthread_cond_wait(&cond, &m);
        }
    }

    std::cout << "[Thread A] Exiting\n";
    return nullptr;
}

// ------------------------------------------------
// Thread B: handles phases 1, 3, 5
// ------------------------------------------------
void* threadB_func(void* /*unused*/) {
    pthread_mutex_lock(&m);

    while (true) {
        if (phase >= 6) {
            pthread_mutex_unlock(&m);
            break;
        }

        if (phase == 1) {
            // B
            pthread_mutex_unlock(&m);
            funcB();
            pthread_mutex_lock(&m);

            phase = 2;
            pthread_cond_broadcast(&cond);
        }
        else if (phase == 3) {
            // D
            pthread_mutex_unlock(&m);
            funcD();
            pthread_mutex_lock(&m);

            phase = 4;
            pthread_cond_broadcast(&cond);
        }
        else if (phase == 5) {
            // B2
            pthread_mutex_unlock(&m);
            funcB2();
            pthread_mutex_lock(&m);

            phase = 6; // done
            pthread_cond_broadcast(&cond);
        }
        else {
            // Not Thread B's turn, wait
            pthread_cond_wait(&cond, &m);
        }
    }

    std::cout << "[Thread B] Exiting\n";
    return nullptr;
}

// ------------------------------------------------
// Implementations of each function
// ------------------------------------------------
void funcA() {
    std::cout << "[A] Running in thread " << pthread_self() << "\n";
    sleep(1);

    // Example: produce aResult
    aResult = 10;
    std::cout << "  [A] -> aResult=" << aResult << "\n";
}

void funcB() {
    std::cout << "[B] Running in thread " << pthread_self() << "\n";
    sleep(1);

    // B depends on aResult
    bResult = aResult + 5;
    std::cout << "  [B] aResult=" << aResult 
              << " => bResult=" << bResult << "\n";
}

void funcE() {
    std::cout << "[E] Running in thread " << pthread_self() << "\n";
    sleep(1);

    // E depends on bResult
    eResult = bResult * 2;
    std::cout << "  [E] bResult=" << bResult 
              << " => eResult=" << eResult << "\n";
}

void funcD() {
    std::cout << "[D] Running in thread " << pthread_self() << "\n";
    sleep(1);

    // D depends on eResult
    dResult = eResult + 100;
    std::cout << "  [D] eResult=" << eResult 
              << " => dResult=" << dResult << "\n";
}

void funcC() {
    std::cout << "[C] Running in thread " << pthread_self() << "\n";
    sleep(1);

    // C depends on dResult
    cResult = dResult - 3;
    std::cout << "  [C] dResult=" << dResult 
              << " => cResult=" << cResult << "\n";
}

void funcB2() {
    std::cout << "[B2] Running in thread " << pthread_self() << "\n";
    sleep(1);

    // B2 depends on cResult
    b2Result = cResult * 10;
    std::cout << "  [B2] cResult=" << cResult 
              << " => b2Result=" << b2Result << "\n";
}

// ------------------------------------------------
// main()
// ------------------------------------------------
int main() {
    pthread_t tA, tB;
    pthread_create(&tA, nullptr, threadA_func, nullptr);
    pthread_create(&tB, nullptr, threadB_func, nullptr);

    // wait for both
    pthread_join(tA, nullptr);
    pthread_join(tB, nullptr);

    std::cout << "\nDone. Final phase=" << phase << "\n";
    std::cout << "aResult=" << aResult 
              << ", bResult=" << bResult
              << ", eResult=" << eResult
              << ", dResult=" << dResult
              << ", cResult=" << cResult
              << ", b2Result=" << b2Result
              << "\n";
    return 0;
}
