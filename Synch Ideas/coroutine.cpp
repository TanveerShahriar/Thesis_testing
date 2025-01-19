// g++ coroutine.cpp -o coroutine -std=c++20 -fcoroutines

#include <iostream>
#include <coroutine>
#include <thread>
#include <chrono>

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

// Simulate a delay
void simulateWork(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

// Function B
Coroutine functionB() {
    std::cout << "Function B: Start execution\n";
    simulateWork(2); // Simulate work
    std::cout << "Function B: End execution\n";

    co_return; // Signal completion
}

// Function A
Coroutine functionA() {
    std::cout << "Function A: Start execution\n";

    // Call Function B and wait for it to complete
    co_await functionB();

    std::cout << "Function A: Resumed execution after Function B\n";
    std::cout << "Function A: End execution\n";

    co_return; // Signal completion
}

int main() {
    auto coro = functionA(); // Start Function A

    while (!coro.handle.done()) {
        coro.handle.resume(); // Resume the coroutine
    }

    return 0;
}