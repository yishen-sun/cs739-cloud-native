#include "state_machine.h"

StateMachine::StateMachine(std::string storage_name) : storage_name(storage_name) {};


void StateMachine::flush_to_disk() {
    ofstream file(storage_name);
    if (!file.is_open()) {
        cout << "Error opening file " << storage_name << " for writing" << endl;
        return;
    }
    for (const auto& entry : value_store_) {
        file << entry.first << " ";
        file << entry.second << " ";
        auto version_infos = version_store_[entry.first];
        for (int i = 0; i < version_infos.size(); i++) {
            string cur_server = version_infos[i].first;
            uint64_t cur_version = version_infos[i].second;
            file << cur_server << "@";
            file << cur_version << " ";
        }
        file << endl;
    }
    file.flush();
    file.close();
}

string StateMachine::get_result(string key) {
    if (value_store_.find(key) != value_store_.end()) {
        return value_store_[key];
    }
    return NO_VALUE;
}

vector<pair<string, uint64_t>> StateMachine::get_server_info(string key) {
    if (version_store_.find(key) != version_store_.end()) {
        return version_store_[key];
    }
    return {};
}

void StateMachine::remove(string key, vector<pair<string, uint64_t>> server_info) {
    value_store_[key] = NO_VALUE;
    version_store_[key] = server_info;
}

void StateMachine::put(string key, string value, vector<pair<string, uint64_t>> server_info) {
    value_store_[key] = value;
    version_store_[key] = server_info;
    flush_to_disk();
}
