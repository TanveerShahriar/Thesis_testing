#include <iostream>
#include <pthread.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

// Global thread and synchronization primitives
pthread_t global_thread;
pthread_t thread2;
std::mutex mtx;
std::condition_variable cv;
bool task_completed = false;  // Indicates when a task is done
bool ready_for_function_c = false; // Indicates when Function B signals for Function C

// Simulate a delay to mimic work
void simulateWork(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

// Function C logic (executed in the same thread as Function A)
void functionC() {
    std::cout << "Function C: Start execution in Thread ID: " << pthread_self() << std::endl;
    simulateWork(2); // Simulate work
    std::cout << "Function C: End execution in Thread ID: " << pthread_self() << std::endl;

    {
        std::lock_guard<std::mutex> lock(mtx);
        task_completed = true; // Mark Function C as done
    }
    cv.notify_one(); // Notify waiting thread
}

// Function B logic (executed in Thread 2)
void* threadFunctionB(void* arg) {
    std::cout << "Function B (Part 1): Start execution in Thread ID: " << pthread_self() << std::endl;
    simulateWork(2); // Simulate work

    // Signal Function A to execute Function C
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Function B: Signaling Function C to execute in Thread ID: " << *(pthread_t*)arg << std::endl;
        ready_for_function_c = true; // Signal readiness for Function C
    }
    cv.notify_one(); // Notify Function A to run Function C

    // Wait for Function C to complete
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return task_completed; });
    }

    std::cout << "Function B (Part 2): Resumed execution in Thread ID: " << pthread_self() << std::endl;
    simulateWork(2); // Simulate work

    {
        std::lock_guard<std::mutex> lock(mtx);
        task_completed = true; // Mark Function B as done
    }
    cv.notify_one(); // Notify waiting thread
    return nullptr;
}

// Function A logic (executed in the global thread)
void* threadFunctionA(void* arg) {
    std::cout << "Function A: Start execution in Thread ID: " << pthread_self() << std::endl;

    // Start Function B in Thread 2
    pthread_create(&thread2, nullptr, threadFunctionB, &global_thread);

    // Wait for Function B to signal Function C
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return ready_for_function_c; });
    }

    // Execute Function C in the global thread
    functionC();

    // Wait for Function B to finish
    pthread_join(thread2, nullptr);

    std::cout << "Function A: Resumed execution after Function B in Thread ID: " << pthread_self() << std::endl;

    // Final logic in Function A
    simulateWork(1); // Simulate work
    std::cout << "Function A: End execution in Thread ID: " << pthread_self() << std::endl;

    return nullptr;
}

int main() {
    // Start Function A in the global thread
    pthread_create(&global_thread, nullptr, threadFunctionA, nullptr);

    // Wait for the global thread to complete
    pthread_join(global_thread, nullptr);

    return 0;
}
