#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <atomic>
#include <random>
#include <vector>
#include <algorithm>

const int SIZE = 8;  // Array size
std::atomic<int>* vec;  // Atomic array

// Global random engine and distribution
std::random_device rd;  
std::mt19937 rng(rd()); 

void initializeArray() {
    vec = new std::atomic<int>[SIZE]; // Dynamically allocate atomic array
    for (int i = 0; i < SIZE; i++) {
        vec[i].store(0);  // Initialize each element to 0
    }
}

int getBalancedRandomIndex() {
    // Step 1: Compute the average of the values
    double sum = 0;
    std::vector<int> values(SIZE);
    
    for (int i = 0; i < SIZE; i++) {
        values[i] = vec[i].load();
        sum += values[i];
    }

    double avg = sum / SIZE;
    double threshold = avg * 0.8; // Consider elements below 80% of the average

    std::vector<int> candidateIndices;

    // Print the entire atomic array
    std::cout << "Current Values in vec: [ ";
    for (int i = 0; i < SIZE; i++) {
        std::cout << values[i] << " ";
    }
    std::cout << "]\n";

    // Step 2: Select indices below the threshold
    for (int i = 0; i < SIZE; i++) {
        if (values[i] <= threshold) {
            candidateIndices.push_back(i);
        }
    }

    // Step 3: If no candidates found, take elements below the median
    if (candidateIndices.empty()) {
        std::vector<int> sortedValues = values;
        std::sort(sortedValues.begin(), sortedValues.end());
        int median = sortedValues[SIZE / 2]; // Approximate median

        for (int i = 0; i < SIZE; i++) {
            if (values[i] <= median) {
                candidateIndices.push_back(i);
            }
        }
    }

    // Print candidate pool with values
    std::cout << "Candidate Pool (Threshold = " << threshold << "): [ ";
    for (int idx : candidateIndices) {
        std::cout << "(" << idx << ": " << values[idx] << ") ";
    }
    std::cout << "]\n";

    // Step 4: Randomly select from candidates
    std::uniform_int_distribution<int> dist(0, candidateIndices.size() - 1);
    return candidateIndices[dist(rng)];
}

int main() {
    initializeArray(); // Initialize dynamically allocated atomic array

    // Manually setting initial values for testing
    // int initialValues[] = {200, 400, 20, 50, 500, 100, 30, 80};
    // for (int i = 0; i < SIZE; i++) {
    //     vec[i].store(initialValues[i]);
    // }

    // Random value generator for addition (1 to 100)
    std::uniform_int_distribution<int> valueDist(1, 100);

    // Simulating multiple selections and updates
    for (int step = 0; step < 10; step++) {
        std::cout << "\nStep " << step + 1 << ":\n";
        
        int selectedIndex = getBalancedRandomIndex();
        int addValue = valueDist(rng); // Generate random value between 1 and 100
        
        vec[selectedIndex].fetch_add(addValue); // Atomic increment
        std::cout << "Selected index = " << selectedIndex 
                  << ", Adding value = " << addValue << "\n";
    }

    delete[] vec;  // Clean up allocated memory
}
