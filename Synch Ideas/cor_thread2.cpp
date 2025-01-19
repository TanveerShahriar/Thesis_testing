#include <iostream>
#include <coroutine>
#include <pthread.h>
#include <chrono>
#include <condition_variable>
#include <mutex>

// Global thread and synchronization primitives
pthread_t global_thread;

// Simulate a delay
void simulateWork(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

// Coroutine return type
struct Coroutine {
    struct promise_type {
        std::mutex mtx;
        std::condition_variable cv;
        bool completed = false;

        Coroutine get_return_object() {
            return Coroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept {
            {
                std::lock_guard<std::mutex> lock(mtx);
                completed = true;
            }
            cv.notify_one(); // Notify resumption
            return {};
        }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    explicit Coroutine(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Coroutine() {
        if (handle) handle.destroy();
    }

    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) const {
        auto& promise = handle.promise();
        std::unique_lock<std::mutex> lock(promise.mtx);
        promise.cv.wait(lock, [&]() { return promise.completed; });
    }
    void await_resume() const noexcept {}
};

// Function C
Coroutine functionC() {
    std::cout << "Function C: Start execution\n";
    simulateWork(2); // Simulate work
    std::cout << "Function C: End execution\n";

    co_return; // Signal completion
}

// Function B
Coroutine functionB() {
    std::cout << "Function B: Start execution\n";

    // Call Function C
    co_await functionC();

    std::cout << "Function B: Resumed execution after Function C\n";
    std::cout << "Function B: End execution\n";

    co_return; // Signal completion
}

// Function A
Coroutine functionA() {
    std::cout << "Function A: Start execution\n";

    // Call Function B
    co_await functionB();

    std::cout << "Function A: Resumed execution after Function B\n";
    std::cout << "Function A: End execution\n";

    co_return; // Signal completion
}

// Thread function to execute a coroutine
void* threadFunction(void* arg) {
    auto coro = static_cast<Coroutine*>(arg);
    while (!coro->handle.done()) {
        coro->handle.resume(); // Resume the coroutine
    }
    return nullptr;
}

int main() {
    // Start Function A in the global thread
    auto mainCoro = functionA();

    // Execute the coroutine in the global thread
    pthread_create(&global_thread, nullptr, threadFunction, &mainCoro);
    pthread_join(global_thread, nullptr);

    return 0;
}
