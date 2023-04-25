#include "gossip_node.h"
#include <cstdlib>
#include <ctime>
#include <vector>

GossipNode::GossipNode(const std::string &node_id, int num_virtual_nodes, const std::string &server_address)
    : node_id_(node_id), server_address_(server_address), ring_(num_virtual_nodes) {
  srand(static_cast<unsigned int>(time(nullptr)));
  channel_ = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  stub_ = gossipnode::GossipNodeService::NewStub(channel_);
}

// ...

grpc::Status GossipNode::JoinNetwork(grpc::ServerContext *context, const gossipnode::JoinRequest *request, gossipnode::JoinResponse *response) {
  // Add the joining node to the ring and membership list
  ring_.addNode(request->node_id(), request->server_address());
  members_heartbeat_list_[request->node_id()] = Timestamp::now();

  // Update the joining node with the current ring data
  auto ring_data = getRingData();
  for (const auto &[hash, addr] : ring_data) {
    auto entry = response->add_ring();
    entry->set_hash(hash);
    entry->set_server_address(addr);
  }

  return grpc::Status::OK;
}

grpc::Status GossipNode::LeaveNetwork(grpc::ServerContext *context, const gossipnode::LeaveRequest *request, gossipnode::LeaveResponse *response) {
  std::string leaving_node_id = request->node_id();
  ring_.removeNode(leaving_node_id);
  network_.erase(leaving_node_id);
  gossip();
  response->set_success(true);
  return grpc::Status::OK;
}

grpc::Status GossipNode::Gossip(grpc::ServerContext *context, const gossipnode::GossipRequest *request, gossipnode::GossipResponse *response) {
//   std::map<size_t, std::string> updated_ring;
//   for (const auto &pair : request->ring_data()) {
//     updated_ring[pair.key()] = pair.value();
//   }
//   updateRing(updated_ring);
//   response->set_success(true);
//   return grpc::Status::OK;
}

grpc::Status GossipNode::UpdateRing(grpc::ServerContext *context, const gossipnode::UpdateRingRequest *request, gossipnode::UpdateRingResponse *response) {
  std::map<size_t, std::string> updated_ring;
  for (const auto &pair : request->ring_data()) {
    updated_ring[pair.key()] = pair.value();
  }
  updateRing(updated_ring);
  response->set_success(true);
  return grpc::Status::OK;
}

grpc::Status GossipNode::ClientGet(grpc::ServerContext *context, const gossipnode::GetRequest *request, gossipnode::GetResponse *response) {
    // read value from 3 replica, receive more than 2 acknowledgement before return to clients
    // data vector clock [[server_name, timestamp] ...] 
    std::string key = request->key();

    // get three replica addr
    // std::vector<name> ring_.get_repicaa(<hash value or value>);
    
    // init result array

    // send grpc request to get data
    // for all addr in vector
    //   if addr == current:
    //         put result 
    //   else:
    //       send peerGet
    //           add result
    // check conflict:
    //    if not conflict -> add the latest version to result array
    //    else -> add all version to result array
    // construct result array

    return grpc::Status::OK;
}

grpc::Status GossipNode::ClientPut(grpc::ServerContext *context, const gossipnode::PutgRequest *request, gossipnode::PutResponse *response) {
  // write value in 3 physical node, receive more than 2 acknowledgement before return to clients
  std::string key = request->key();
  std::string value = request->data
  
  // 1. get three replica addr
  // 2. <send grpc request to put data>
  // for all addr in vector:
  //    if addr == current:
  //         put result
  //    else:
  //         send peerPut
  // if more than 2 success:
  //   return success
  return grpc::Status::OK;
}

grpc::Status GossipNode::PeerPut(grpc::ServerContext *context, const gossipnode::PeerPutgRequest *request, gossipnode::PeerPutResponse *response) {
  // put result into rings
  return grpc::Status::OK;
}

grpc::Status GossipNode::PeerGet(grpc::ServerContext *context, const gossipnode::PeerGetRequest *request, gossipnode::PeerGetResponse *response) {
  // get result, put result into request results.
  return grpc::Status::OK;
}

void GossipNode::peerPut(const std::string server, const std::string key, const std::string value) {

};

void GossipNode::peerGet() {

}

void GossipNode::updateRing(const std::map<size_t, std::string>& updated_ring) {
//   ring_ = ConsistentHashingRing(ring_.num_virtual_nodes_);
//   for (const auto& entry : updated_ring) {
//     ring_.addNode(entry.second);
//   }
}

void GossipNode::joinNetwork(const Address &other_node_address) {
  // Connect to the other node and send a join request
  auto stub = gossipnode::GossipNodeService::NewStub(grpc::CreateChannel(other_node_address, grpc::InsecureChannelCredentials()));
  grpc::ClientContext context;
  gossipnode::JoinRequest request;
  gossipnode::JoinResponse response;

  request.set_node_id(ring_.getNodeId());
  request.set_server_address(server_address_);

  grpc::Status status = stub->JoinNetwork(&context, request, &response);

  if (status.ok()) {
    updateRing(response.ring());
  } else {
    std::cerr << "Failed to join network: " << status.error_message() << std::endl;
  }
}

void GossipNode::gossip() {
  // Select random nodes from the membership list to gossip with
  std::vector<Address> random_nodes;

  // TODO: Select random nodes from the membership list

  gossipnode::GossipRequest request;
  gossipnode::GossipResponse response;

  // Prepare the gossip request with the local membership list
  for (const auto &[node_id, timestamp] : members_heartbeat_list_) {
    auto entry = request.add_members();
    entry->set_node_id(node_id);
    entry->set_timestamp(timestamp.time_since_epoch().count());
  }

  for (const Address &addr : random_nodes) {
    grpc::ClientContext context;
    auto stub = stubs_[addr].get();
    grpc::Status status = stub->Gossip(&context, request, &response);

    if (!status.ok()) {
      std::cerr << "Failed to gossip with " << addr << ": " << status.error_message() << std::endl;
    }
  }
}