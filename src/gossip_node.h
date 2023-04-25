#ifndef GOSSIP_NODE_H
#define GOSSIP_NODE_H

#include "consistent_hashing_ring.h"
#include "gossip_node.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <map>
#include <string>
#include <chrono>

const int RPELICA_N = 3;
const int W_COUNT = 2;
const int R_COUNT = 2;

class GossipNode : public gossipnode::GossipNodeService::Service {
public:
  GossipNode(const std::string &node_id, int num_virtual_nodes, const std::string &server_address);


  void joinNetwork(const std::string &other_node_address);
  void leaveNetwork();

  // grpc gossip will call this function
  void updateRing(const std::map<size_t, std::string> &updated_ring);
  
  // send members_heartbeat_list_ to other node
  void gossip();
  // send heartbeat to a set of random node

  // gRPC service method implementations
  grpc::Status JoinNetwork(grpc::ServerContext *context, const gossipnode::JoinRequest *request, gossipnode::JoinResponse *response) override;
  grpc::Status LeaveNetwork(grpc::ServerContext *context, const gossipnode::LeaveRequest *request, gossipnode::LeaveResponse *response) override;

  
  // receive members_heartbeat_list_ from other node
  // 1. if I think a node is down (from my own members_heartbeat_list_), and the members_heartbeat_list_ that I just received also indicates that node down
  // 1.1 then I mark the node is down, and update ring. If not, the node is not down.
  // 2. if I receive a new node, add to the list
  grpc::Status Gossip(grpc::ServerContext *context, const gossipnode::GossipRequest *request, gossipnode::GossipResponse *response) override;
  
  // receive ring metadata, and update
  grpc::Status UpdateRing(grpc::ServerContext *context, const gossipnode::UpdateRingRequest *request, gossipnode::UpdateRingResponse *response) override;
  
  // write value in 3 physical node, receive more than 2 acknowledgement before return to clients
  grpc::Status Put(grpc::ServerContext *context, const gossipnode::PutgRequest *request, gossipnode::PutResponse *response) override;
      
  // read value from 3 replica, receive more than 2 acknowledgement before return to clients
  // data vector clock [[server_name, timestamp] ...] 
  grpc::Status Get(grpc::ServerContext *context, const gossipnode::GetRequest *request, gossipnode::GetResponse *response) override;
      
  grpc::Status Delete(grpc::ServerContext *context, const gossipnode::DeleteRequest *request, gossipnode::DeleteResponse *response) override;

/*
bucket name: db
store: real kv pair

bucket name: vector clock
/node_id_1
    key, timestamp
/node_id_2
    key, timestamp

*/

private:
// std::string node_id_;
std::string server_address_;
ConsistentHashingRing ring_;
// std::shared_ptr<grpc::Channel> channel_;
unordered_map<std::string, std::shared_ptr<gossipnode::GossipNodeService::Stub>> stubs_; // key: server_addr, value: stub_
void read_exists_servers(); // read configuration to initialize grpc stubs to all other servers
void update_exists_servers(); // if the server is permernantly removed, modify the configuration.
//std::unique_ptr<gossipnode::GossipNodeService::Stub> stub_;


// std::map<std::string, std::string> network_;
// std::map<size_t, std::string> getRingData(); // ?????

// membership heartbeat list
std::unordered_map<std::string, std::chrono::high_resolution_clock> members_heartbeat_list_;

// storage interface
std::unordered_map<size_t, std::string> storage;
};
#endif // GOSSIP_NODE_H