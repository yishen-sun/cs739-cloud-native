#include "client_grpc.h"

string NO_MASTER_YET = "NO_MASTER_YET";

KeyValueStoreClient::KeyValueStoreClient(string config_path, string assigned_port) : config_path(config_path), assigned_port(assigned_port),
state_machine_("client_storage.txt") {
    grpc::ChannelArguments channel_args;
    channel_args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, INT_MAX);
    channel_ = grpc::CreateCustomChannel(assigned_port, grpc::InsecureChannelCredentials(), channel_args);
    stub_ = gossipnode::GossipNodeService::NewStub(channel_);
    read_server_config();
    // random_pick_server();
}

bool KeyValueStoreClient::Put(const string& key, const string& value) {
    grpc::ClientContext context;
    gossipnode::PutRequest request;
    gossipnode::PutResponse response;
    request.set_key(key);
   
    auto& data = *(request.mutable_data());
    data.set_value(value);

    string result;
    bool success;
    do {
        success = Get(key, result); 
    } while (success != true);

    for (const auto& version_pair : state_machine_.get_version(key)) {
      auto& version_info = *(data.add_version_info());
      version_info.set_server(version_pair.first);
      version_info.set_version(version_pair.second);
    }
    
    
    // context.set_deadline(chrono::system_clock::now() + chrono::milliseconds(100));
    grpc::Status status = stub_->ClientPut(&context, request, &response);
    if (status.ok()) {
        if (response.ret() == gossipnode::PutReturn::OK) {
            cout << "Put return status is ok, response true" << endl;
        } else if (response.ret() == gossipnode::PutReturn::FAILED) {
            return false;
        } else if (response.ret() == gossipnode::PutReturn::NOT_COORDINATOR) {
            // update channel to the coordinator
            update_channel_to_coordinator(response.coordinator());
            return Put(key, value);
        }
        return true;

    } else {
        cerr << "Put RPC failed: " << status.error_message() << endl;
        return false;
    }
    return true;
}

bool KeyValueStoreClient::Get(const string& key, string& result) {
    grpc::ClientContext context;
    gossipnode::GetRequest request;
    gossipnode::GetResponse response;

    request.set_key(key);
    // context.set_deadline(chrono::system_clock::now() + chrono::milliseconds(100));
    grpc::Status status = stub_->ClientGet(&context, request, &response);
    vector<pair<string, vector<pair<string, uint64_t>>>> potential_result;
    vector<pair<string, uint64_t>> vector_clock;
    if (status.ok()) {
        for (const auto& data : response.get_res_data()) {
            string value = data.value();
            vector_clock.clear();
            for (const auto& server_version : data.version_info()) {
                vector_clock.push_back(make_pair(server_version.server(), server_version.version()));
            }
            potential_result.push_back(make_pair(value, vector_clock));
        }
        reconcile(potential_result, key, result);
        return true;
        
    } else {
        cerr << "Get RPC failed: " << status.error_message() << endl;
        return false;
    }

}


bool KeyValueStoreClient::read_server_config() {
    // file format:
    // <name>/<addr> e.g. A/0.0.0.0:50001
    ifstream infile(config_path);
    string line;
    while (getline(infile, line)) {
        size_t pos = line.find('/');
        if (pos != string::npos) {
            string key = line.substr(0, pos);
            string value = line.substr(pos + 1, line.size() - pos - 1);
            server_config[key] = value;
        }
    }
    return true;
}

void KeyValueStoreClient::random_pick_server() {
    auto random_server =
        next(begin(server_config), rand_between(0, server_config.size() - 1));
    grpc::ChannelArguments channel_args;
    channel_args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, INT_MAX);
    cout << random_server->second << endl;
    channel_ = grpc::CreateCustomChannel(random_server->second, grpc::InsecureChannelCredentials(),
                                         channel_args);
    stub_ = gossipnode::GossipNodeService::NewStub(channel_);
}

void KeyValueStoreClient::update_channel_to_coordinator(string coordinator) {
    grpc::ChannelArguments channel_args;
    channel_args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, INT_MAX);
    
    channel_ = grpc::CreateCustomChannel(server_config[coordinator], grpc::InsecureChannelCredentials(),
                                         channel_args);
    stub_ = gossipnode::GossipNodeService::NewStub(channel_);
}

int KeyValueStoreClient::rand_between(int start, int end) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(start, end);
    return dis(gen);
}


void KeyValueStoreClient::reconcile(vector<pair<string, vector<pair<string, uint64_t>>>> conflict_versions, string key, string& result) {
    if (conflict_versions.size() == 0) {
        result = "";
        return;
    }
    if (conflict_versions.size() == 1) {
        result = conflict_versions[0].first;
        state_machine_.put(key, result, conflict_versions[0].second);
        return;
    }
    int i = 0;
    for (const auto& potential_pair : conflict_versions) {
        cout << "Potential result including" << endl;
        cout << i << ": " << potential_pair.first << endl;
        i++;
    }
    cout << "Select which version do you want to keep" << endl;
    int selected;
    vector<pair<string, uint64_t>> reconcile_version = state_machine_.reconcile_version(conflict_versions);

    while (true) {
        cin >> selected;
        if (selected >= 0 && selected < conflict_versions.size()) {
            result = conflict_versions[selected].first;
            state_machine_.put(key, result, reconcile_version);
            Put(key, result);
            return;
        }
        else {
            cout << "Invalid input. Please enter a number between 0 and " << conflict_versions.size() - 1 << endl;
        }
    }
}
