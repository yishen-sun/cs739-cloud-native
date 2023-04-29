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


// using grpc::Channel;
// using grpc::ClientContext;
// using grpc::Status;
// using kvraft::GetRequest;
// using kvraft::GetResponse;
// using kvraft::HelloReply;
// using kvraft::HelloRequest;
// using kvraft::KVRaft;
// using kvraft::PutRequest;
// using kvraft::PutResponse;

class KeyValueStoreClient {
   public:
    KeyValueStoreClient(string config_path, string asigned_port);

    bool Put(const string& key, const string& value);

    bool Get(const string& key, string& result);

   private:
    unique_ptr<gossipnode::GossipNodeService::Stub> stub_;
    shared_ptr<grpc::Channel> channel_;
    string config_path;
    string assigned_port;
    unordered_map<string, string> server_config;  // k = name A, v = addr:port 0.0.0.0:50001
    bool read_server_config();
    void random_pick_server();
    int rand_between(int start, int end);
    StateMachine state_machine_;
    void reconcile(vector<pair<string, vector<pair<string, uint64_t>>>> conflict_versions, string key, string& result);
};