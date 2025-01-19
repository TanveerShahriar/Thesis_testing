#include <iostream>
#include <pthread.h>

using namespace std;
pthread_t globalThread;

struct addParams
{
    int a;
    int b;
    int res;
};

struct funcParams
{
    int a;
    int b;
    int res;
};

int add(int a, int b)
{
    return a + b;
}

void* threadadd(void* params)
{
    struct addParams* args = (struct addParams*) params;
    int result = add(args->a, args->b);
    args->res = result;

    return nullptr;
}

int func(int a, int b){
    int c = a - b;

    int d;

    struct addParams* addArgs0 = (struct addParams*) malloc(sizeof(struct addParams));
    addArgs0->a = a;
    addArgs0->b = b;

    pthread_create(&globalThread, NULL, &threadadd, addArgs0);
    int t0 = addArgs0->res;

    d = t0;

    free(addArgs0);

    return c * d;
}

void* threadfunc(void* params)
{
    struct funcParams* args = (struct funcParams*) params;
    int result = func(args->a, args->b);
    args->res = result;

    return nullptr;
}

int main() {
    int e;

    struct funcParams* funcArgs1 = (struct funcParams*) malloc(sizeof(struct funcParams));
    funcArgs1->a = 5;
    funcArgs1->b = 3;

    pthread_create(&globalThread, NULL, &threadfunc, funcArgs1);
    int t1 = funcArgs1->res;

    e = t1;

    cout << e << endl;

    free(funcArgs1);

    pthread_join(globalThread, NULL);
    return 0;
}
