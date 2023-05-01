#pragma once

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include "gossip_node.grpc.pb.h"
#include "state_machine.h"
using namespace std;

class Admin {
   public:
    Admin(string config_path, string state_machine_name);

    bool Cmd(const string& addr, const string& cmd);

    void printAllNodeStatus();
   private:
    string config_path_;
    StateMachine state_machine_;
    unordered_map<string, bool> server_status_;  // k = name A, v = addr:port 0.0.0.0:50001
    unordered_set<string> enum_cmd_;
    std::unordered_map<std::string, std::shared_ptr<gossipnode::GossipNodeService::Stub>> stubs_; // key: server_addr, value: stub_
    bool read_server_config();
    
};