#include <iostream>
#include <stack>
#include <string>

// Identify different functions with an enum
enum class FunctionId {
    FUNC_A,
    FUNC_B
};

// A structure representing one 'stack frame'
struct Frame {
    FunctionId funcId;  // Which function is this frame for?
    int pc;             // 'Program counter' or instruction index within this function.
    
    // Possible local variables:
    int localVar;       // Just an example local variable
};

// We'll define a helper function to "call" another function:
// i.e., push a new frame onto our call stack.
void callFunction(std::stack<Frame>& callStack, FunctionId id) {
    Frame newFrame{ id, 0, 0 };
    callStack.push(newFrame);
}

// We'll define a helper function to simulate "return" from the current function:
// i.e., pop the top frame from the stack.
void returnFromFunction(std::stack<Frame>& callStack) {
    callStack.pop();
}

// Main interpreter loop
int main() 
{
    // Our manual call stack
    std::stack<Frame> callStack;
    
    // Initially, we "call" funcA (like main calling A).
    callFunction(callStack, FunctionId::FUNC_A);
    
    // Keep going until there's nothing left on the call stack
    while (!callStack.empty()) {
        // Grab a reference to the top of the stack (current function).
        Frame &currentFrame = callStack.top();
        
        switch (currentFrame.funcId) {
            case FunctionId::FUNC_A: {
                // Simulate the code inside funcA using a switch on currentFrame.pc
                switch (currentFrame.pc) {
                    case 0:
                        std::cout << "[funcA] Starting.\n";
                        // Next instruction: call funcB
                        currentFrame.pc++;
                        // We "call" funcB by pushing a funcB frame, but we don't advance
                        // pc in A after the call. We'll do that after B returns.
                        callFunction(callStack, FunctionId::FUNC_B);
                        break;
                        
                    case 1:
                        // If we get here, that means funcB must have returned
                        // so we continue in funcA after the call
                        std::cout << "[funcA] funcB returned, continuing.\n";
                        currentFrame.pc++;
                        break;
                        
                    case 2:
                        // End of funcA
                        std::cout << "[funcA] Ending.\n";
                        // Return from funcA
                        returnFromFunction(callStack);
                        break;
                        
                    default:
                        // We should never get here for this example
                        returnFromFunction(callStack);
                        break;
                }
                break;
            }
            
            case FunctionId::FUNC_B: {
                // A trivial function that just prints something and returns
                switch (currentFrame.pc) {
                    case 0:
                        std::cout << "  [funcB] Doing some work...\n";
                        currentFrame.pc++;
                        break;
                    case 1:
                        std::cout << "  [funcB] Done. Returning.\n";
                        // Return from funcB
                        returnFromFunction(callStack);
                        break;
                    default:
                        // Shouldn't happen in our example
                        returnFromFunction(callStack);
                        break;
                }
                break;
            }
        }
    }
    
    std::cout << "All done. The stack is empty.\n";
    return 0;
}
