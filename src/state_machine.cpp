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


int StateMachine::check_conflict_version(const vector<pair<string, uint64_t>>& incoming_vector, 
                                        const vector<pair<string, uint64_t>>& existing_vector) {
    //TODO: Bad performance !!!
    // [["Server_a", 1], ["Server_b", 2], ...]
    //  2: conflict
    //  1: incoming is latest than existing
    //  0: incoming is in the same time version as existing
    // -1: incomding is earlier than exisiting
    // corner case: empty existing

    if (existing_vector.size() == 0) {
        return 1;
    }
    if (incoming_vector.size() == 0) {
        return -1;
    }
    int res = 0;
    unordered_set<string> incoming_set;
    for (const auto& incoming_pair : incoming_vector) incoming_set.insert(incoming_pair.first);
    for (const auto& incoming_pair : incoming_vector) { 
        for (const auto& existing_pair : existing_vector) {
            if (incoming_set.find(existing_pair.first) == incoming_set.end()) {
                return 2;
            }
            if (incoming_pair.first == existing_pair.first && incoming_pair.second < existing_pair.second) {
                if (res == 1) return 2; // conflict because previous comparison show that incoming is the latest
                res = -1;
            }
            if (incoming_pair.first == existing_pair.first && incoming_pair.second > existing_pair.second) {
                if (res == -1) return 2; // conflict because previous comparison show that incoming is the earlier
                res = 1;
            }
        }
    }    

    return res;
}


vector<pair<string, vector<pair<string, uint64_t>>>> StateMachine::remove_duplicate_data(
    //TODO: Bad performance !!!
    const vector<pair<string, vector<pair<string, uint64_t>>>>& version_vectors) {
    
    vector<pair<string, vector<pair<string, uint64_t>>>> unique_vectors;
    for (auto& check_vector : version_vectors) {
        bool is_duplicate = false;
        for (auto& unique_vector : unique_vectors) {
            if (check_vector.first == unique_vector.first && check_vector.second == unique_vector.second) {
                is_duplicate = true;
                break;
            }
        }
        if (!is_duplicate) unique_vectors.push_back(check_vector);
    }
    return unique_vectors;
}

vector<pair<string, vector<pair<string, uint64_t>>>> StateMachine::remove_unconflict_data(
    //TODO: Bad performance !!!
    const vector<pair<string, vector<pair<string, uint64_t>>>>& original_vectors) {
    
    vector<pair<string, vector<pair<string, uint64_t>>>> possible_conflict_data;
    bool is_old_data;
    for (int i = 0; i < original_vectors.size(); i++) {
        is_old_data = false;
        for (int j = 0; j < original_vectors.size(); j ++) {
            if (i != j) {
                if (check_conflict_version(original_vectors[i].second, original_vectors[j].second) < 0) {
                    is_old_data = true;
                    break;
                }
            }
        }
        if (is_old_data == false) {
            possible_conflict_data.push_back(original_vectors[i]);
        }
    }
    return possible_conflict_data;
}


vector<pair<string, vector<pair<string, uint64_t>>>> StateMachine::get_latest_data(
    const vector<pair<string, vector<pair<string, uint64_t>>>>& original_vectors) {
        // remove duplicate
    vector<pair<string, vector<pair<string, uint64_t>>>> latest_data;
    latest_data = remove_duplicate_data(original_vectors);
    latest_data = remove_unconflict_data(latest_data);
    return latest_data;
}