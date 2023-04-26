#pragma once

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include "gossip_node.grpc.pb.h"


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
    KeyValueStoreClient(std::string config_pat, std::string asigned_port);

    bool Put(const std::string& key, const std::string& value);

    bool Get(const std::string& key, std::string& result);

   private:
    std::unique_ptr<gossipnode::GossipNodeService::Stub> stub_;
    std::shared_ptr<grpc::Channel> channel_;
    std::string config_path;
    std::string assigned_port;
    std::unordered_map<std::string, std::string> server_config;  // k = name A, v = addr:port 0.0.0.0:50001
    bool read_server_config();
    void random_pick_server();
    int rand_between(int start, int end);
};