#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

int phase = -1;

int funcE_a;
int funcE_b;
int funcE_res;

int funcD_a;
int funcD_b;
int funcD_res;

void funcE(){
    cout << "Inside E " << pthread_self() << endl;
    funcE_res = 2 * funcE_a + 2 * funcE_b;
}

void funcD_1(){
    cout << "Start D " << pthread_self() << endl;
    sleep(1);

    funcE_a = 2;
    funcE_b = 3;
}

void funcD_2(){
    int res = funcE_res;
    cout << "End D " << res << " " << pthread_self() << endl;
    funcD_res = res - funcD_a + funcD_b;
}

void funcC(){
    cout << "Start C " << pthread_self() << endl;

    sleep(1);
    funcD_a = 4;
    funcD_b = 5;

    cout << "End C " << pthread_self() << endl;
}

void funcB(){
    cout << "Start B " << pthread_self() << endl;

    sleep(1);

    cout << "End B " << pthread_self() << endl;
}

void funcA(){
    cout << "Start A " << pthread_self() << endl;

    sleep(1);

    cout << "End A " << pthread_self() << endl;
}

void* thread1_func(void* /*unused*/){
    while (true){
        if (phase > 4){
            break;
        }
        else if (phase == 0){
            funcA();
            funcC();
            phase += 1;
        }
        else if (phase == 1){
            while (true){
                if (phase != 1) break;
            }
        }
        else if (phase == 2){
            while (true){
                if (phase != 2) break;
            }
        }
        else if (phase == 3){
            funcE();
            phase += 1;
        }
        else if (phase == 4){
            while (true){
                if (phase != 4) break;
            }
        }
    }
    
    return nullptr;
}

void* thread2_func(void* /*unused*/){
    while (true){
        if (phase > 4){
            break;
        }
        else if (phase == 0){
            funcB();
            phase += 1;
        }
        else if (phase == 1){
            while (true){
                if (phase != 1) break;
            }
        }
        else if (phase == 2){
            funcD_1();
            phase += 1;
        }
        else if (phase == 3){
            while (true){
                if (phase != 3) break;
            }
        }
        else if (phase == 4){
            funcD_2();
            phase += 1;
        }
    }

    return nullptr;
}

int main(){
    pthread_t thread1, thread2;
    phase = 0;
    pthread_create(&thread1, nullptr, thread1_func, nullptr);
    pthread_create(&thread2, nullptr, thread2_func, nullptr);

    // wait for both
    pthread_join(thread1, nullptr);
    pthread_join(thread2, nullptr);

    return 0;
}