#include <iostream>
#include <thread>
#include <fstream>
#include <chrono>
#include "throughput.h"

int main(int argc, char** argv) {
    if (argc != 8) {
        std::cout << "you must provide four arguments: config_path server_addr, iteration, key_length, value_length, client_num, behavior"
                  << std::endl;
        std::cout << "Usage: ./throughput one_server_config.txt 0.0.0.0:50001 10000 1000 1000 5 put" << std::endl;
        return 0;
    }

    int num_clients = stoi(argv[6]); // Number of parallel clients to run
    std::vector<std::thread> threads(num_clients);
    std::vector<ThroughputTest> tests;
    std::string behavior(argv[7]);
    std::string config_path(argv[1]);
    std::string server_addr(argv[2]);

    // Prepare the test cases
    for (int i = 0; i < num_clients; i++) {
        tests.push_back(ThroughputTest(config_path, server_addr, behavior));
        
        threads[i] = std::thread(&ThroughputTest::prepare_testcase, &tests[i], stoi(argv[3]), stoi(argv[4]), stoi(argv[5]));
        
        // tests[i] = ThroughputTest(config_path, server_addr, behavior);
        // tests[i].prepare_testcase(stoi(argv[3]), stoi(argv[4]), stoi(argv[5]));
    }


    // Wait for all threads to finish
    for (int i = 0; i < num_clients; i++) {
        threads[i].join();
    }

    // Record the start time
    auto start_time = std::chrono::high_resolution_clock::now();

    // Create and run the clients in separate threads
    if (behavior == "put") {
        for (int i = 0; i < num_clients; i++) {
            threads[i] = std::thread(&ThroughputTest::run_put_operation, &tests[i], stoi(argv[3]));
        }
    } else {
        for (int i = 0; i < num_clients; i++) {
            threads[i] = std::thread(&ThroughputTest::run_get_operation, &tests[i], stoi(argv[3]));
        }
    }
    

    // Wait for all threads to finish
    for (int i = 0; i < num_clients; i++) {
        threads[i].join();
    }

    // Record the end time and calculate the elapsed time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    // Print the total time and append it to the output file
    std::cout << "Total time: " << duration << " milliseconds" << std::endl;
    std::ofstream outfile;
    outfile.open("throughput_result_" + behavior + "_" + argv[6] + "_clients_.txt", std::ios_base::app);
    outfile << argv[3] << " pairs, key: " << argv[4] << ", value: " << argv[5] << std::endl;
    outfile << "Total time: " << duration << " milliseconds" << std::endl;
    outfile.close();

    return 0;
}