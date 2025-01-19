#include <iostream>
#include <vector>
#include <functional>

// Define a call stack using std::function<void()>
std::vector<std::function<void()>> callStack;

// Generic function to add a function to the stack
template <typename Func, typename... Args>
void addToCallStack(Func&& func, Args&&... args) {
    // Use a lambda to bind arguments
    callStack.emplace_back([=]() {
        func(args...);
    });
}

// Execute the call stack
void executeCallStack() {
    while (!callStack.empty()) {
        // Get the last function in the stack
        auto func = callStack.back();
        callStack.pop_back(); // Remove it from the stack
        func(); // Execute it
    }
}

// Example functions with different signatures
void printMessage(const std::string& message) {
    std::cout << "Message: " << message << std::endl;
}

int add(int a, int b) {
    std::cout << "Sum: " << (a + b) << std::endl;
    return a + b; // Note: Return value is ignored
}

void greet(const std::string& name, int age) {
    std::cout << "Hello, " << name << "! You are " << age << " years old." << std::endl;
}

int main() {
    // Add functions with various signatures to the stack
    addToCallStack(printMessage, "Welcome to the call stack!");
    addToCallStack(add, 5, 10);
    addToCallStack(greet, "Alice", 25);
    addToCallStack(add, 5, 10);

    // Execute the call stack
    executeCallStack();

    return 0;
}
