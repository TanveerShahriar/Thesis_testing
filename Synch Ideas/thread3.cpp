#include <iostream>
#include <vector>
#include <functional>
#include <any>

using namespace std;

// Define a call stack using function<any()>
vector<function<any()>> callStack;

// Generic function to add a function to the stack
template <typename Func, typename... Args>
void addToCallStack(Func&& func, Args&&... args) {
    // Use a lambda to bind arguments and return an `any` result
    callStack.emplace_back([=]() -> any {
        return func(args...);
    });
}

// Execute the call stack and return the result of the last function executed
any executeCallStack() {
    any lastResult;
    while (!callStack.empty()) {
        // Get the last function in the stack
        auto func = callStack.back();
        callStack.pop_back(); // Remove it from the stack
        lastResult = func(); // Execute it and store the result
    }
    return lastResult;
}

// Function to add two numbers
int add(int a, int b) {
    return a + b;
}

// Function with logic
int func(int a, int b) {
    int c = a - b;

    // Add the `add` function to the call stack
    addToCallStack(add, 5, 3);

    // Execute the stack to get the result of `add`
    int d = any_cast<int>(executeCallStack());

    return c * d;
}

int main() {
    // Add the initial function to the stack
    addToCallStack(func, 5, 3);

    // Execute the stack and retrieve the result of `func`
    int result = any_cast<int>(executeCallStack());

    // Print the result
    cout << "Final Result: " << result << endl;

    return 0;
}
