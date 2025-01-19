#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>

using namespace std;

pthread_t thread1, thread2;
vector<int> stack1, stack2;

bool t1_b = false, t2_b = false;

int funcE_a;
int funcE_b;
int funcE_res;

int funcD_a;
int funcD_b;
int funcD_res;

void funcE(){
    cout << "Inside E " << pthread_self() << endl;
    funcE_res = 2 * funcE_a + 2 * funcE_b;
    stack2.push_back(5);
    t2_b = true;
}

void funcD_1(){
    cout << "Start D " << pthread_self() << endl;


    funcE_a = 2;
    funcE_b = 3;
    stack1.push_back(6);
    t1_b = true;
}

void funcD_2(){
    int res = funcE_res;
    cout << "End D " << res << " " << pthread_self() << endl;
    funcD_res = res - funcD_a + funcD_b;
}

void funcC(){
    cout << "Start C " << pthread_self() << endl;


    funcD_a = 4;
    funcD_b = 5;
    stack2.push_back(4);
    t2_b = true;


    cout << "End C " << pthread_self() << endl;
}

void funcB(){
    cout << "Start B " << pthread_self() << endl;


    stack1.push_back(3);
    t1_b = true;
    sleep(2);

    cout << "End B " << pthread_self() << endl;
}

void funcA(){
    cout << "Start A " << pthread_self() << endl;


    stack2.push_back(2);
    t2_b = true;


    cout << "End A " << pthread_self() << endl;
}

void* thread1_func(void* /*unused*/){
    s1:
    while (!t1_b) {  }
    while (!stack1.empty()){
        int f = stack1.back();
        stack1.pop_back();

        if (f == 1) {
            funcA();
            t1_b = false;
            goto s1;
        }
        else if (f == 3) {
            funcC();
            t1_b = false;
            goto s1;
        }
        else if (f == 6) {
            funcE();
            t1_b = false;
            goto s1;
        }
    }
    return nullptr;
}

void* thread2_func(void* /*unused*/){
    s2:
    while (!t2_b) { }
    while (!stack2.empty()){
        int f = stack2.back();
        stack2.pop_back();

        if (f == 2) {
            funcB();
            t2_b = false;
            goto s2;
        }
        else if (f == 4) {
            funcD_1();
            t2_b = false;
            goto s2;
        }
        else if (f == 5) {
            funcD_2();
            t2_b = false;
            goto s2;
        }
    }
    return nullptr;
}

int main(){
    stack1.push_back(1);
    t1_b = true;
    pthread_create(&thread1, nullptr, thread1_func, nullptr);
    pthread_create(&thread2, nullptr, thread2_func, nullptr);

    // wait for both
    pthread_join(thread1, nullptr);
    pthread_join(thread2, nullptr);

    return 0;
}