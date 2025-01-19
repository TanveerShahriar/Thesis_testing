#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

int funcE(int a, int b){
    cout << "Inside E " << pthread_self() << endl;
    return 2 * a + 2 * b;
}

int funcD(int a, int b){
    cout << "Start D " << pthread_self() << endl;
    sleep(1);

    int res = funcE(2, 3);

    cout << "End D " << res << " " << pthread_self() << endl;
    return res - a + b;
}

void funcC(){
    cout << "Start C " << pthread_self() << endl;

    sleep(1);
    funcD(4, 5);

    cout << "End C " << pthread_self() << endl;
}

void funcB(){
    cout << "Start B " << pthread_self() << endl;

    sleep(1);
    funcC();

    cout << "End B " << pthread_self() << endl;
}

void funcA(){
    cout << "Start A " << pthread_self() << endl;

    sleep(1);
    funcB();

    cout << "End A " << pthread_self() << endl;
}

int main(){
    funcA();
    return 0;
}