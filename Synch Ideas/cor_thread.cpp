// g++ cor_thread.cpp -o cor_thread -std=c++20 -fcoroutines
// g++ cor_thread2.cpp -o cor_thread2 -std=c++20 -fcoroutines

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

// Simulate a delay
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

    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) const { handle.resume(); }
    void await_resume() const noexcept {}
};

// Function B
void* threadFunction(void* arg) {
    if (running_function_b) {
        std::cout << "Function B: Start execution\n";
        simulateWork(2); // Simulate work
        std::cout << "Function B: End execution\n";

        {
            std::lock_guard<std::mutex> lock(mtx);
            task_completed = true;
        }
        cv.notify_one(); // Notify Function A
    }

    return nullptr;
}

Coroutine functionB() {
    // Switch `global_thread` to execute Function B
    running_function_b = true;
    pthread_create(&global_thread, nullptr, threadFunction, nullptr);
    pthread_join(global_thread, nullptr);

    co_return; // Signal completion
}

// Function A
void* threadFunctionA(void*) {
    auto coroA = []() -> Coroutine {
        std::cout << "Function A: Start execution\n";

        // Call Function B and wait for it to complete
        co_await functionB();

        // Wait for the signal that Function B has completed
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [] { return task_completed; });
        }

        std::cout << "Function A: Resumed execution after Function B\n";
        std::cout << "Function A: End execution\n";

        co_return; // Signal completion
    };

    auto coro = coroA();
    while (!coro.handle.done()) {
        coro.handle.resume(); // Resume the coroutine
    }

    return nullptr;
}

int main() {
    // Start Function A in `global_thread`
    pthread_create(&global_thread, nullptr, threadFunctionA, nullptr);
    pthread_join(global_thread, nullptr);

    return 0;
}
