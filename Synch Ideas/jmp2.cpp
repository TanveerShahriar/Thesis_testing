#include <iostream>
#include <csetjmp>
#include <vector>
#include <unistd.h>
#include <pthread.h>

// Wrap jmp_buf in a struct to avoid direct array type issues
struct JmpBufWrapper {
    jmp_buf buffer;  // jmp_buf is typically an array type
};

static std::vector<JmpBufWrapper*> jumpStack1;

pthread_t thread1, thread2;

void funcC() {
    std::cout << "S Func C " << pthread_self() << std::endl;
    sleep(2);
    std::cout << "E Func C " << pthread_self() << std::endl;

    // longjmp on the top of the stack
    longjmp(jumpStack1.back()->buffer, 1);
}

void funcB() {
    std::cout << "S Func B " << pthread_self() << std::endl;
    sleep(2);

    funcC();

    std::cout << "E Func B " << pthread_self() << std::endl;
}

void* threadFunctionB(void* arg) {
    funcB();

    return nullptr;
}

void funcA() {
    std::cout << "S Func A " << pthread_self() << std::endl;
    sleep(2);

    // Dynamically allocate a JmpBufWrapper
    auto* jb = new JmpBufWrapper;
    jumpStack1.push_back(jb);

    int ret = setjmp(jb->buffer);
    if (ret == 0) {
        pthread_create(&thread2, nullptr, threadFunctionB, nullptr);
    }

    // Cleanup after we're done
    delete jumpStack1.back();
    jumpStack1.pop_back();

    std::cout << "E Func A " << pthread_self() << std::endl;
}

void* threadFunctionA(void* arg) {
    funcA();

    return nullptr;
}

int main() {
    pthread_create(&thread1, nullptr, threadFunctionA, nullptr);

    pthread_join(thread1, nullptr);
    return 0;
}