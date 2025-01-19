#include <iostream>
#include <queue>
#include <functional>
#include <pthread.h>
#include <condition_variable>
#include <mutex>

using namespace std;

// Task definition
struct Task {
    function<void()> func;     // Function to execute
    mutex taskMutex;           // Mutex for synchronization
    condition_variable taskCV; // Condition variable for task completion
    bool done = false;         // Flag to indicate completion
};

// Task queue and synchronization primitives
queue<Task*> taskQueue;
mutex queueMutex;
condition_variable queueCV;
bool stopThread = false;

// Global worker thread
pthread_t globalThread;

// Worker thread function
void* workerThread(void* args) {
    while (true) {
        Task* task = nullptr;

        // Fetch the next task from the queue
        {
            unique_lock<mutex> lock(queueMutex);
            queueCV.wait(lock, [] { return !taskQueue.empty() || stopThread; });

            if (stopThread && taskQueue.empty()) break;

            task = taskQueue.front();
            taskQueue.pop();
        }

        // Execute the task
        if (task) {
            task->func();

            // Mark task as done and notify waiting threads
            {
                lock_guard<mutex> taskLock(task->taskMutex);
                task->done = true;
            }
            task->taskCV.notify_one();
        }
    }

    return nullptr;
}

// Function to enqueue tasks
void enqueueTask(Task* task) {
    lock_guard<mutex> lock(queueMutex);
    taskQueue.push(task);
    queueCV.notify_one();
}

// Add function
int add(int a, int b) {
    return a + b;
}

// Func function
int func(int a, int b) {
    int result = 0;

    // Create a task for func
    Task funcTask;
    funcTask.func = [&]() {
        int c = a - b;

        // Create a nested task for add
        Task addTask;
        addTask.func = [&]() {
            int addResult = add(a, b);
            result = c * addResult;
        };

        enqueueTask(&addTask);

        // Wait for the add task to complete
        unique_lock<mutex> addLock(addTask.taskMutex);
        addTask.taskCV.wait(addLock, [&]() { return addTask.done; });
    };

    enqueueTask(&funcTask);

    // Wait for the func task to complete
    unique_lock<mutex> funcLock(funcTask.taskMutex);
    funcTask.taskCV.wait(funcLock, [&]() { return funcTask.done; });

    return result;
}

int main() {
    // Start the global worker thread
    pthread_create(&globalThread, nullptr, workerThread, nullptr);

    // Call func using the global thread
    int result = func(5, 3);
    cout << "Result: " << result << endl;

    // Stop the global thread
    {
        lock_guard<mutex> lock(queueMutex);
        stopThread = true;
        queueCV.notify_one();
    }
    pthread_join(globalThread, nullptr);

    return 0;
}