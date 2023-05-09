#include "throughput.h"

ThroughputTest::ThroughputTest(std::string config_path, std::string leader_addr, std::string behavior): 
        client_(config_path, leader_addr), behavior(behavior){

}


// Create a random string of the specified length
std::string ThroughputTest::random_string(size_t length) {
    static const std::string CHARACTERS =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    static std::random_device rd;
    static std::mt19937 generator(rd());

    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string random_string;

    for (size_t i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}

void ThroughputTest::prepare_testcase(int num_iterations, int key_length, int value_length) {
    std::string key;
    std::string value;
    while (test_case.size() < num_iterations) {
        key = random_string(key_length);
        value = random_string(value_length);
        test_case[key] = value;
        if (behavior == "get") {
            client_.Put(key, value, false);
        }
    }
}

// Run the Put operation for a specified number of iterations
void ThroughputTest::run_put_operation(int num_iterations) {
    std::cout << "Start to run put operation" << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    bool res = true;
    int i = 0;
    for (const auto& entry : test_case) {
        i++;
        auto before_crash_time = std::chrono::high_resolution_clock::now();
        res = (client_.Put(entry.first, entry.second, false) || res);
        auto after_crash_time = std::chrono::high_resolution_clock::now();
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    if (res == false) {
        std::cout << "Failed to execute commands" << std::endl;
        exit(0);
    }

    std::cout << "Put operation completed:" << num_iterations << " kv pairs in " << duration << " microseconds" << std::endl;
}

// Run the Get operation for a specified number of iterations
void ThroughputTest::run_get_operation(int num_iterations) {
    std::cout << "Start to run get operation" << std::endl;
    std::string result;
    auto start_time = std::chrono::high_resolution_clock::now();
    bool res = true;
    for (const auto& entry : test_case) {
        res = (client_.Get(entry.first, result) || res);
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    
    if (res == false) {
        std::cout << "Failed to execute commands" << std::endl;
        exit(0);
    }
    std::cout << "Get operation completed " << num_iterations << " kv pairs in " << duration << " microseconds" << std::endl;
}

