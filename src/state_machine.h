#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <vector>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <unordered_set>
#include <sstream>

using namespace std;

const string NO_VALUE = "__NO_VALUE_";

class StateMachine {
private:
    string storage_name;
    string nodes_config_;
    unordered_map<string, string> value_store_;
    unordered_map<string, vector<pair<string, uint64_t>>> version_store_;
public:
    StateMachine(string storage_name, string nodes_config_);
    unordered_set<string> get_nodes_config();
    void write_nodes_config(std::unordered_set<std::string> config);
    void put(string key, string value, vector<pair<string, uint64_t>> server_info);
    void remove(string key/*, vector<pair<string, uint64_t>> server_info*/);
    vector<pair<string, uint64_t>> get_version(string key);
    string get_value(string key);
    void flush_to_disk();
    void read_file();
    vector<string> get_all_keys();
    int check_conflict_version(const vector<pair<string, uint64_t>>& incoming_version, const vector<pair<string, uint64_t>>& existing_version);
    // bool check_conflict_versions(const vector<pair<string, vector<pair<string, uint64_t>>>>& data_version);
    vector<pair<string, vector<pair<string, uint64_t>>>> remove_duplicate_data(const vector<pair<string, vector<pair<string, uint64_t>>>>& version_vectors);
    vector<pair<string, vector<pair<string, uint64_t>>>> get_latest_data(const vector<pair<string, vector<pair<string, uint64_t>>>>& original_vectors);
    vector<pair<string, vector<pair<string, uint64_t>>>> remove_unconflict_data(const vector<pair<string, vector<pair<string, uint64_t>>>>& original_vectors);
    
    vector<pair<string, uint64_t>> update_version(const vector<pair<string, uint64_t>>& original_version, const string& server_name);
    vector<pair<string, uint64_t>> reconcile_version(vector<pair<string, vector<pair<string, uint64_t>>>> conflict_versions);
    void print_version(vector<pair<string, uint64_t>> versions);
};

#endif