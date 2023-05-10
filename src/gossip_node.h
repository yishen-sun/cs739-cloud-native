#ifndef GOSSIP_NODE_H
#define GOSSIP_NODE_H

#include "consistent_hashing_ring.h"
#include "state_machine.h"
#include "gossip_node.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <map>
#include <string>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <future>


const int REPLICA_N = 3;
const int W_COUNT = 2;
const int R_COUNT = 2;
constexpr int HEARTBEAT_INTERVAL = 5000;
constexpr int CHECK_ALIVE_INTERVAL = 10000;
const bool FIXED_CONFIG_TEST = false;
const int MAX_WAIT_TIME = 10;

#define RESET "\033[0m"
#define BLACK "\033[30m"   /* Black */
#define RED "\033[31m"     /* Red: error */
#define GREEN "\033[32m"   /* Green: gossip */
#define YELLOW "\033[33m"  /* Yellow */
#define BLUE "\033[34m"    /* Blue: admin */
#define MAGENTA "\033[35m" /* Magenta: peer */
#define CYAN "\033[36m"    /* Cyan: client */
#define WHITE "\033[37m"   /* White */

class GossipNode : public gossipnode::GossipNodeService::Service {
public:
  GossipNode(const std::string &node_id, int num_virtual_nodes, const std::string &server_address, const std::string& config_path);


  bool joinNetwork();
  void leaveNetwork();
  
  // send members_heartbeat_list_ to other node
  void gossip();
  // send heartbeat to a set of random node

  int peerPut(const std::string peer_server, const std::string key, const std::string value,const std::vector<std::pair<std::string, uint64_t>> vector_clock);
  int peerGet(const std::string peer_server, const std::string key, std::string& value, std::vector<std::pair<std::string, uint64_t>>& vector_clock);

  // gRPC service method implementations
  grpc::Status JoinNetwork(grpc::ServerContext *context, const gossipnode::JoinRequest *request, gossipnode::JoinResponse *response) override;
  grpc::Status LeaveNetwork(grpc::ServerContext *context, const gossipnode::LeaveRequest *request, gossipnode::LeaveResponse *response) override;

  
  // receive members_heartbeat_list_ from other node
  // 1. if I think a node is down (from my own members_heartbeat_list_), and the members_heartbeat_list_ that I just received also indicates that node down
  // 1.1 then I mark the node is down, and update ring. If not, the node is not down.
  // 2. if I receive a new node, add to the list
  grpc::Status Gossip(grpc::ServerContext *context, const gossipnode::GossipRequest *request, gossipnode::GossipResponse *response) override;
  
  // write value in 3 physical node, receive more than 2 acknowledgement before return to clients
  grpc::Status ClientPut(grpc::ServerContext *context, const gossipnode::PutRequest *request, gossipnode::PutResponse *response) override;
  // read value from 3 replica, receive more than 2 acknowledgement before return to clients
  // data vector clock [[server_name, timestamp] ...] 
  grpc::Status ClientGet(grpc::ServerContext *context, const gossipnode::GetRequest *request, gossipnode::GetResponse *response) override;
  // grpc::Status ClientDelete(grpc::ServerContext *context, const gossipnode::DeleteRequest *request, gossipnode::DeleteResponse *response) override;

  grpc::Status PeerPut(grpc::ServerContext *context, const gossipnode::PeerPutRequest *request, gossipnode::PeerPutResponse *response) override;
  grpc::Status PeerGet(grpc::ServerContext *context, const gossipnode::PeerGetRequest *request, gossipnode::PeerGetResponse *response) override;

  grpc::Status AdminCmd(grpc::ServerContext *context, const gossipnode::AdminCmdRequest *request, gossipnode::AdminCmdResponse *response) override;
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
std::string node_id_;
std::string server_address_;
ConsistentHashingRing ring_;
StateMachine state_machine_;
// std::shared_ptr<grpc::Channel> channel_;
std::unordered_map<std::string, std::shared_ptr<gossipnode::GossipNodeService::Stub>> stubs_; // key: server_addr, value: stub_
void read_exists_servers(); // read configuration to initialize grpc stubs to all other servers
std::unordered_set<std::string> read_server_config_update_stubs_(); // if the server is permernantly removed, modify the configuration.
bool is_coordinator(vector<string>& quorum_member);
//std::unique_ptr<gossipnode::GossipNodeService::Stub> stub_;
std::vector<std::string> getJoinTransferKey(const std::string &node_id, int num_replicas);
std::vector<std::string> getLeaveTransferKey(ConsistentHashingRing& old_ring, const std::string &node_id, int num_replicas);
std::unordered_map<std::string, std::vector<std::string>> node_transfer_keys_;
// std::map<std::string, std::string> network_;
// std::map<size_t, std::string> getRingData(); // ?????
public:
// membership heartbeat list
std::unordered_map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> members_heartbeat_list_;

// storage interface
std::unordered_map<size_t, std::string> storage_;

// alive status
bool alive_;

void print_version(vector<pair<string, uint64_t>> versions);
string assigned_coordinator(vector<string>& quorum_member);
};
#endif // GOSSIP_NODE_H