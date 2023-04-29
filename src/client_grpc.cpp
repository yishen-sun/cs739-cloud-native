#include "client_grpc.h"

std::string NO_MASTER_YET = "NO_MASTER_YET";

KeyValueStoreClient::KeyValueStoreClient(std::string config_path, std::string assigned_port) : config_path(config_path), assigned_port(assigned_port) {
    grpc::ChannelArguments channel_args;
    channel_args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, INT_MAX);
    channel_ = grpc::CreateCustomChannel(assigned_port, grpc::InsecureChannelCredentials(), channel_args);
    stub_ = gossipnode::GossipNodeService::NewStub(channel_);
    read_server_config();
    // random_pick_server();
}

bool KeyValueStoreClient::Put(const std::string& key, const std::string& value) {
    // grpc::ClientContext context;
    // gossipnode::PutRequest request;
    // gossipnode::PutResponse response;
    // auto& data = *(request.mutable_data());
    // data.set_key(key);
    // data.set_value(value);
    
    // // context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(100));
    // grpc::Status status = stub_->ClientPut(&context, request, &response);
    // if (status.ok()) {
    //     if (response.success()) {
    //         std::cout << "Put return status is ok, response true" << std::endl;
    //     } else {
    //         std::cout << "Put return status is ok, response false" << std::endl;
    //     }
    //     return true;

    // } else {
    //     std::cerr << "Put RPC failed: " << status.error_message() << std::endl;
    //     return false;
    // }
    return true;
}

bool KeyValueStoreClient::Get(const std::string& key, std::string& result) {
    grpc::ClientContext context;
    gossipnode::GetRequest request;
    gossipnode::GetResponse response;

    request.set_key(key);
    // context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(100));
    grpc::Status status = stub_->ClientGet(&context, request, &response);
    std::vector<std::pair<std::string, uint64_t>> vector_clock;
    if (status.ok()) {
        for (const auto& data : response.get_res_data()) {
            std::string value = data.value();
            std::cout << "Value: " << value << std::endl;
            for (const auto& server_version : data.version_info()) {
                std::string server = server_version.server();
                uint64_t version = server_version.version();
                vector_clock.push_back(std::make_pair(server, version));
                std::cout << "  Server: " << server << ", Version: " << version << std::endl;
            }
        }
        return true;
        
    } else {
        std::cerr << "Get RPC failed: " << status.error_message() << std::endl;
        return false;
    }

}


bool KeyValueStoreClient::read_server_config() {
    // file format:
    // <name>/<addr> e.g. A/0.0.0.0:50001
    std::ifstream infile(config_path);
    std::string line;
    while (std::getline(infile, line)) {
        size_t pos = line.find('/');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1, line.size() - pos - 1);
            server_config[key] = value;
        }
    }
    return true;
}

void KeyValueStoreClient::random_pick_server() {
    auto random_server =
        std::next(std::begin(server_config), rand_between(0, server_config.size() - 1));
    grpc::ChannelArguments channel_args;
    channel_args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, INT_MAX);
    std::cout << random_server->second << std::endl;
    channel_ = grpc::CreateCustomChannel(random_server->second, grpc::InsecureChannelCredentials(),
                                         channel_args);
    stub_ = gossipnode::GossipNodeService::NewStub(channel_);
}

int KeyValueStoreClient::rand_between(int start, int end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(start, end);
    return dis(gen);
}