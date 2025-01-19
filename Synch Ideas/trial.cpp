#include <iostream>
#include <coroutine>
#include <pthread.h>
#include <chrono>
#include <condition_variable>
#include <mutex>

// Global thread and synchronization primitives
pthread_t global_thread;
std::mutex mtx;
std::condition_variable cv;
bool task_completed = false;
bool running_function_b = false;
bool running_function_c = false;

// Simulate a delay to mimic work
void simulateWork(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

// Coroutine return type
struct Coroutine {
    struct promise_type {
        Coroutine get_return_object() {
            return Coroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    explicit Coroutine(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Coroutine() {
        if (handle) handle.destroy();
    }

    void resume() {
        if (handle && !handle.done()) {
            handle.resume();
        }
    }
};

// Function C logic
void* threadFunctionC(void* arg) {
    if (running_function_c) {
        std::cout << "Function C: Start execution\n";
        simulateWork(2); // Simulate work
        std::cout << "Function C: End execution\n";

        {
            std::lock_guard<std::mutex> lock(mtx);
            task_completed = true;
        }
        cv.notify_one(); // Notify waiting thread
    }

    return nullptr;
}

// Function C coroutine
Coroutine functionC() {
    running_function_c = true;

    // Start Function C in a separate thread
    pthread_create(&global_thread, nullptr, threadFunctionC, nullptr);
    pthread_join(global_thread, nullptr);

    co_return; // Coroutine signals completion
}

// Function B logic
void* threadFunction(void* arg) {
    if (running_function_b) {
        std::cout << "Function B: Start execution\n";
        simulateWork(2); // Simulate work

        // Explicitly invoke Function C coroutine
        auto coroC = functionC();
        while (!coroC.handle.done()) {
            coroC.resume(); // Resume coroutine until done
        }

        // Wait for Function C completion signal
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return task_completed; });
        }

        std::cout << "Function B: End execution\n";

        {
            std::lock_guard<std::mutex> lock(mtx);
            task_completed = true;
        }
        cv.notify_one(); // Notify waiting thread
    }

    return nullptr;
}

// Function B coroutine
Coroutine functionB() {
    running_function_b = true;

    // Start Function B in a separate thread
    pthread_create(&global_thread, nullptr, threadFunction, nullptr);
    pthread_join(global_thread, nullptr);

    co_return; // Coroutine signals completion
}

// Function A logic
void* threadFunctionA(void* arg) {
    std::cout << "Function A: Start execution\n";

    // Explicitly invoke Function B coroutine
    auto coroB = functionB();
    while (!coroB.handle.done()) {
        coroB.resume(); // Resume coroutine until done
    }

    // Wait for Function B completion signal
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return task_completed; });
    }

    std::cout << "Function A: Resumed execution after Function B\n";
    std::cout << "Function A: End execution\n";

    return nullptr;
}

// Function A coroutine
Coroutine functionA() {
    // Start Function A in a separate thread
    pthread_create(&global_thread, nullptr, threadFunctionA, nullptr);
    pthread_join(global_thread, nullptr);

    co_return; // Coroutine signals completion
}

int main() {
    // Explicitly invoke Function A coroutine
    auto coroA = functionA();
    while (!coroA.handle.done()) {
        coroA.resume(); // Resume coroutine until done
    }

    return 0;
}
