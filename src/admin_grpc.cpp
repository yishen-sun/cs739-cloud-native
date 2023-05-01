#include "admin_grpc.h"

Admin::Admin(string config_path, string state_machine_name) : config_path_(config_path),
state_machine_(state_machine_name, config_path) {
    read_server_config();
    enum_cmd_.emplace("Ping");
    enum_cmd_.emplace("JoinNetwork");
    enum_cmd_.emplace("LeaveNetwork");
}

bool Admin::Cmd(const string& addr, const string& cmd) {
    grpc::ClientContext context;
    gossipnode::AdminCmdRequest request;
    gossipnode::AdminCmdResponse response;
    if (server_status_.count(addr) == 0 || enum_cmd_.count(cmd) == 0) {
        cout << "addr or cmd wrong" << endl;
        return false;
    }
    request.set_cmd(cmd);
    context.set_deadline(chrono::system_clock::now() + chrono::milliseconds(100));
    grpc::Status status = stubs_[addr]->AdminCmd(&context, request, &response);
    cout << "sending " << cmd <<" to: " << addr << endl;

    if (!status.ok()) {
      std::cerr << "AdminCmd RPC failed: " << status.error_message() << std::endl;
      server_status_[addr] = false;
      return false;
    }
    if (cmd == "Ping") {
        if (response.success()) server_status_[addr] = true;
        else server_status_[addr] = false;
    } else if (cmd == "JoinNetwork") {
        if (response.success()) server_status_[addr] = true;
        else server_status_[addr] = false;
    } else if (cmd == "LeaveNetwork") {
        if (response.success()) server_status_[addr] = false;
        else server_status_[addr] = true;
    }
    return true;
}

bool Admin::read_server_config() {
    unordered_set<string> config = state_machine_.get_nodes_config();
    for (const auto& server_addr: config) {
        grpc::ChannelArguments channel_args;
        channel_args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, INT_MAX);
        std::shared_ptr<grpc::Channel> channel_ = grpc::CreateCustomChannel(server_addr, grpc::InsecureChannelCredentials(), channel_args);
        stubs_[server_addr] = gossipnode::GossipNodeService::NewStub(channel_);
        server_status_[server_addr] = false;
        cout << "success init " << server_addr << endl;
    }
    return true;
}

void Admin::printAllNodeStatus() {
    std::cout << "printAllNodeStatus" << std::endl;
    for (const auto& kv : server_status_) {
        std::cout << "Node: " << kv.first << " Status: " << kv.second << std::endl;
    }
}