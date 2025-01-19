#include <iostream>
#include <pthread.h>

using namespace std;

int add(int a, int b){
    return a + b;
}

int func(int a, int b){
    int c = a - b;

    int d = add(a, b);

    return c * d;
}

int main() {
    cout << func(5, 3) << endl;
    return 0;
}
