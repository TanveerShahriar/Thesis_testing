#include <iostream>
#include <coroutine>
#include <pthread.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stack>

// Global thread and synchronization primitives
pthread_t global_thread;
std::mutex mtx;
std::condition_variable cv;
bool task_completed = false;

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

    bool isDone() const {
        return handle.done();
    }
};

// Function C logic
void* threadFunctionC(void* arg) {
    std::cout << "Function C: Start execution\n";
    std::cout << "Thread ID: " << pthread_self() << std::endl;
    simulateWork(2); // Simulate work
    std::cout << "Function C: End execution\n";

    {
        std::lock_guard<std::mutex> lock(mtx);
        task_completed = true;
    }
    cv.notify_one(); // Notify waiting thread
    return nullptr;
}

// Function C coroutine
Coroutine functionC() {
    // Execute Function C in the global thread
    pthread_create(&global_thread, nullptr, threadFunctionC, nullptr);
    pthread_join(global_thread, nullptr);
    co_return; // Signal completion
}

// Function B logic
void* threadFunctionB(void* arg) {
    std::cout << "Function B: Start execution\n";
    std::cout << "Thread ID: " << pthread_self() << std::endl;
    simulateWork(2); // Simulate work

    // Call Function C coroutine
    auto coroC = functionC();
    while (!coroC.isDone()) {
        coroC.resume(); // Resume coroutine
    }

    std::cout << "Function B: End execution\n";

    {
        std::lock_guard<std::mutex> lock(mtx);
        task_completed = true;
    }
    cv.notify_one(); // Notify waiting thread
    return nullptr;
}

// Function B coroutine
Coroutine functionB() {
    // Execute Function B in the global thread
    pthread_create(&global_thread, nullptr, threadFunctionB, nullptr);
    pthread_join(global_thread, nullptr);
    co_return; // Signal completion
}

// Function A logic
void functionA() {
    std::cout << "Function A: Start execution\n";

    // Call Function B coroutine
    auto coroB = functionB();
    while (!coroB.isDone()) {
        coroB.resume(); // Resume coroutine
    }

    std::cout << "Function A: End execution\n";
}

// Global thread execution function
void* globalThreadFunction(void* arg) {
    std::cout << "Thread ID: " << pthread_self() << std::endl;
    // Function A executes entirely in the global thread
    functionA();
    return nullptr;
}

int main() {
    // Start the global thread
    pthread_create(&global_thread, nullptr, globalThreadFunction, nullptr);
    pthread_join(global_thread, nullptr);

    return 0;
}
