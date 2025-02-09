#include <iostream>
#include "obfuscator.hpp"

using namespace std;

void funcE(int thread_idx, int param_index) {
    cout << "[funcE] Start on thread " << this_thread::get_id() << endl;
    funcE_params[param_index].funcE_return = 2 * funcE_params[param_index].funcE_a + 2 * funcE_params[param_index].funcE_b;
    funcE_params[param_index].funcE_done = true;
    vec[thread_idx].fetch_sub(funcE_lines);
}

void funcD(int thread_idx, int param_index) {
    cout << "[funcD] Start on thread " << this_thread::get_id() << endl;
    this_thread::sleep_for(chrono::seconds(1));

    int index;
    {
        unique_lock<mutex> lock(mutexes[thread_idx]);
        if (funcE_params_index_pool.empty()) {
            index = funcE_params.size();
            funcE_params.emplace_back();
        }
        else {
            index = funcE_params_index_pool.front();
            funcE_params_index_pool.pop();
        }
        funcE_params[index] = {2, 3, 0, false};
    }
    
    pushToThread(FUNC_E, funcE_lines, index);
    while (!funcE_params[index].funcE_done) {
        if (!queues[thread_idx].empty()) execute(thread_idx);
    }
    
    int res = funcE_params[index].funcE_return;
    funcD_params[param_index].funcD_return = res - funcD_params[param_index].funcD_a + funcD_params[param_index].funcD_b;
    
    cout << "[funcD] End on thread " << res << " " << this_thread::get_id() << endl;
    funcE_params_index_pool.push(index);
    funcD_params[param_index].funcD_done = true;
    vec[thread_idx].fetch_sub(funcD_lines);
}