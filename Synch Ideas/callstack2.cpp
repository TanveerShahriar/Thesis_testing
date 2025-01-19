#include <iostream>
#include <vector>
#include <functional>
#include <any>

// Define a call stack entry
struct Call {
    std::function<std::any()> func; // Callable that returns a std::any
    std::string description;        // Optional description for debugging
    std::any result;                // To store the result after execution
};

// Define the call stack
std::vector<Call> callStack;

// Generic function to add a function to the stack
template <typename Func, typename... Args>
void addToCallStack(const std::string& desc, Func&& func, Args&&... args) {
    // Use a lambda to bind arguments and capture the return value
    callStack.emplace_back(Call{
        [=]() -> std::any {
            if constexpr (std::is_void_v<std::invoke_result_t<Func, Args...>>) {
                func(args...); // Call the function
                return {};     // Return an empty std::any for void functions
            } else {
                return func(args...); // Call the function and return the result
            }
        },
        desc,
        {} // Initialize result as empty
    });
}

// Execute the call stack and handle nested calls
void executeCallStack() {
    while (!callStack.empty()) {
        auto& call = callStack.back();
        std::cout << "Executing: " << call.description << std::endl;
        call.result = call.func(); // Execute the function and store the result
        callStack.pop_back();
    }
}

// Example functions
std::string concatenate(const std::string& s1, const std::string& s2) {
    std::string result = s1 + s2;
    std::cout << "Concatenated result: " << result << std::endl;
    return result;
}

int add(int a, int b) {
    // Nested function call to concatenate
    addToCallStack("Concatenate strings in add", concatenate, "Sum: ", std::to_string(a + b));

    // Execute the nested call (or defer it to the main stack execution)
    executeCallStack();

    std::cout << "Returning addition result: " << (a + b) << std::endl;
    return a + b;
}

int main() {
    // Add functions to the stack
    addToCallStack("Add 3 and 4 with nested call", add, 3, 4);

    // Execute the call stack
    executeCallStack();

    return 0;
}
