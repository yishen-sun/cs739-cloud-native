#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <vector>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
using namespace std;

const string NO_VALUE = "__NO_VALUE_";

class StateMachine {
private:
    string storage_name;
    unordered_map<string, string> value_store_;
    unordered_map<string, vector<pair<string, uint64_t>>> version_store_;
public:
    StateMachine(string storage_name);
    void put(string key, string value, vector<pair<string, uint64_t>> server_info);
    void remove(string key, vector<pair<string, uint64_t>> server_info);
    vector<pair<string, uint64_t>> get_server_info(string key);
    string get_result(string key);
    void flush_to_disk();
};

#endif