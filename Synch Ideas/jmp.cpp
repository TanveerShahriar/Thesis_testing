#include <iostream>
#include <csetjmp>
#include <vector>
#include <unistd.h>

// Wrap jmp_buf in a struct to avoid direct array type issues
struct JmpBufWrapper {
    jmp_buf buffer;  // jmp_buf is typically an array type
};

static std::vector<JmpBufWrapper*> jumpStack1;

void funcC() {
    std::cout << "S Func C" << std::endl;
    sleep(2);
    std::cout << "E Func C" << std::endl;

    // longjmp on the top of the stack
    longjmp(jumpStack1.back()->buffer, 1);
}

void funcB() {
    std::cout << "S Func B" << std::endl;
    sleep(2);

    funcC();

    std::cout << "E Func B" << std::endl;
}

void funcA() {
    std::cout << "S Func A" << std::endl;
    sleep(2);

    // Dynamically allocate a JmpBufWrapper
    auto* jb = new JmpBufWrapper;
    jumpStack1.push_back(jb);

    int ret = setjmp(jb->buffer);
    if (ret == 0) {
        funcB();
    }

    // Cleanup after we're done
    delete jumpStack1.back();
    jumpStack1.pop_back();

    std::cout << "E Func A" << std::endl;
}

int main() {
    funcA();
    return 0;
}