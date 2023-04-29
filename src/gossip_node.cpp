#include "gossip_node.h"

GossipNode::GossipNode(const std::string &node_id, int num_virtual_nodes, const std::string &server_address)
    : node_id_(node_id), server_address_(server_address), ring_(node_id, num_virtual_nodes), state_machine_(node_id + "storage.txt") {
  // srand(static_cast<unsigned int>(time(nullptr)));
  // channel_ = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  // stub_ = gossipnode::GossipNodeService::NewStub(channel_);
  read_server_config_update_stubs_();
}


bool GossipNode::read_server_config_update_stubs_() {
    // file format:
    // <name>/<addr> e.g. A/0.0.0.0:50001
    // TODO: link to S3
    
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
    stubs_.clear();
    for (const auto& pair : server_config) {
        const std::string& curr_name = pair.first;
        const std::string& curr_addr = pair.second;
        if (curr_name != name) {
            stubs_[curr_addr] =
                GossipNode::NewStub(grpc::CreateChannel(curr_addr, grpc::InsecureChannelCredentials()));
        }
    }
    return true;
}

std::vector<std::string> GossipNode::getTransferKey(const std::string &node_id, , int num_replicas) {
  ConsistentHashingRing tmp_ring(ring_);
  tmp_ring.add
  std::vector<std::string> all_keys = state_machine_.get_all_keys();
  std::vector<std::string> filtered_keys;
  for (auto keys : all_keys) {
    if (tmp_ring.checkDataBelonging(keys, num_replicas) == false) filtered_keys.push_back(keys);
  }
  return filtered_keys;
}

bool GossipNode::joinNetwork(const std::string &node_address) {
  bool flag = true;
  for (auto i : stubs_) {
    grpc::ClientContext context;
    gossipnode::JoinRequest request;
    gossipnode::JoinResponse response;
    request.set_node_id(node_address);
    grpc::Status status = i.second->JoinNetwork(&context, request, &response);
    if (!status.ok()) {
      std::cerr << "joinNetwork RPC failed: " << i.first << " " << status.error_message() << std::endl;
      flag = false;
    } else {
      if (!response.success()) std::cout << "response is not success TODO retry " << i.first << std::endl;
      flag = false;
    }
  }

  if (flag) {
    flag = updateRing();
  }
  return flag;
  // gossip(); // necessary?
}

grpc::Status GossipNode::JoinNetwork(grpc::ServerContext *context, const gossipnode::JoinRequest *request, gossipnode::JoinResponse *response) {
  std::vector<std::string> keys = getTransferKey(request->node_id());
  response->set_success(true);
  for (auto key : keys) {
    int res = peerPut(request->node_id(), key, state_machine_.get_value(key), state_machine_.get_version(key));
    if (res < 0) response->set_success(false);
  }
  return grpc::Status::OK;
}

bool GossipNode::updateRing() {
  bool flag = true;
  for (auto i : stubs_) {
    grpc::ClientContext context;
    gossipnode::JoinRequest request;
    gossipnode::JoinResponse response;
    request.set_node_id(node_address);
    grpc::Status status = i.second->UpdateRing(&context, request, &response);
    if (!status.ok()) {
      std::cerr << "UpdateRing RPC failed: " << i.first << " " << status.error_message() << std::endl;
      flag = false;
    } else {
      if (!response.success()) std::cout << "response is not success TODO retry " << i.first << std::endl;
      flag = false;
    }
  }
  if (flag) {
    // Add myself to the ring and update membership list
    ring_.addNode(node_address);
    // members_heartbeat_list_[node_address] = std::chrono::high_resolution_clock::now();
    std::cout << "Successfully join the network" << std::endl;
  }
  return flag;
}

grpc::Status GossipNode::UpdateRing(grpc::ServerContext *context, const gossipnode::UpdateRingRequest *request, gossipnode::UpdateRingResponse *response) {
  // Add the joining node to the ring and update membership list
  ring_.addNode(request->node_id());
  //members_heartbeat_list_[request->node_id()] = std::chrono::high_resolution_clock::now();
  response->set_success(true);
  return grpc::Status::OK;
}

grpc::Status GossipNode::LeaveNetwork(grpc::ServerContext *context, const gossipnode::LeaveRequest *request, gossipnode::LeaveResponse *response) {
  // std::string leaving_node_id = request->node_id();
  // ring_.removeNode(leaving_node_id);
  // network_.erase(leaving_node_id);
  // gossip();
  // response->set_success(true);
  return grpc::Status::OK;
}

grpc::Status GossipNode::Gossip(grpc::ServerContext *context, const gossipnode::GossipRequest *request, gossipnode::GossipResponse *response) {
//   std::map<size_t, std::string> updated_ring;
//   for (const auto &pair : request->ring_data()) {
//     updated_ring[pair.key()] = pair.value();
//   }
//   updateRing(updated_ring);
//   response->set_success(true);
  return grpc::Status::OK;
}



grpc::Status GossipNode::ClientGet(grpc::ServerContext *context, const gossipnode::GetRequest *request, gossipnode::GetResponse *response) {
    // read value from 3 replica, receive more than 2 acknowledgement before return to clients
    // data vector clock [[server_name, timestamp] ...] 
    std::string key = request->key();

    // get three replica addr
    // std::vector<name> ring_.get_repicaa(<hash value or value>);
    std::vector<std::string> replica_servers = ring_.getReplicasNodes(key, RPELICA_N);
    // init result array
    std::vector<std::pair<std::string, std::vector<std::pair<std::string, uint64_t>>>> results;
    // format: [D1, [[s1, 1], [s2, 2], [s3, 1]]]
    int success_cnt = 0;
    for (auto server : replica_servers) {
      if (server == server_address_) {
          // state machine
          results.push_back(std::make_pair(state_machine_.get_value(key), state_machine_.get_version(key)));
      } else {
        std::string value;
        std::vector<std::pair<std::string, uint64_t>> vector_clock;
        if (peerGet(server, key, value, vector_clock) == 0) {
          results.push_back(std::make_pair(value, vector_clock));
          success_cnt += 1;
        }
      }
    }

    for (auto result : results) {
      gossipnode::Data *add_data = response->add_get_res_data();
      add_data->set_value(result.first);
      for (auto version_info : result.second) {
        gossipnode::ServerVersion* server_version = add_data->add_version_info();
        server_version->set_server(version_info.first);
        server_version->set_version(version_info.second);
        
      }
    }
   

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
    if (success_cnt >= R_COUNT) {
      return grpc::Status::OK;
    }
    return grpc::Status::CANCELLED;
}

grpc::Status GossipNode::ClientPut(grpc::ServerContext *context, const gossipnode::PutRequest *request, gossipnode::PutResponse *response) {
  // write value in 3 physical node, receive more than 2 acknowledgement before return to clients
  std::string key = request->data().key();
  std::string value = request->data().value();
  
  std::vector<std::string> replica_servers = ring_.getReplicasNodes(key, RPELICA_N);
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

grpc::Status GossipNode::PeerPut(grpc::ServerContext *context, const gossipnode::PeerPutRequest *request, gossipnode::PeerPutResponse *response) {
  // put result into rings
  return grpc::Status::OK;
}

grpc::Status GossipNode::PeerGet(grpc::ServerContext *context, const gossipnode::PeerGetRequest *request, gossipnode::PeerGetResponse *response) {
  // get result, put result into request results.
  return grpc::Status::OK;
}

int GossipNode::peerPut(const std::string peer_server, const std::string key, const std::string value, const vector<pair<string, uint64_t>> version) {
    auto stub = stubs_[peer_server];
    grpc::ClientContext context;
    gossipnode::PeerPutRequest request;
    gossipnode::PeerPutResponse response;

    auto& data = *(request.mutable_data());
    data.set_key(key);
    data.set_value(value);
    
    
    grpc::Status status = stub->PeerPut(&context, request, &response);
    if (status.ok()) {
      return 0;
    } else {
      return -1;
    }
};

int GossipNode::peerGet(const std::string peer_server, const std::string key, std::string& value, std::vector<std::pair<std::string, uint64_t>>& vector_clock) {
    auto stub = stubs_[peer_server];
    grpc::ClientContext context;
    gossipnode::PeerGetRequest request;
    gossipnode::PeerGetResponse response;

    request.set_key(key);
    grpc::Status status = stub->PeerGet(&context, request, &response);
    if (!status.ok()) {
      std::cerr << "peerGet RPC failed: " << status.error_message() << std::endl;
      // Handle error
      return 1;
    }

    gossipnode::Data data = response.data();
    value = data.value();
    std::cout << "Value: " << value << std::endl;
    
    for (const auto& server_version : data.version_info()) {
      std::string server = server_version.server();
      uint64_t version = server_version.version();
      vector_clock.push_back(std::make_pair(server, version));
      std::cout << "  Server: " << server << ", Version: " << version << std::endl;
    }
  
    return 0;
}





void GossipNode::gossip() {
  // // Select random nodes from the membership list to gossip with
  // std::vector<std::string> random_nodes;

  // // TODO: Select random nodes from the membership list

  // gossipnode::GossipRequest request;
  // gossipnode::GossipResponse response;

  // // Prepare the gossip request with the local membership list
  // for (const auto &[node_id, timestamp] : members_heartbeat_list_) {
  //   auto entry = request.add_members();
  //   entry->set_node_id(node_id);
  //   entry->set_timestamp(timestamp.time_since_epoch().count());
  // }

  // for (const std::string &addr : random_nodes) {
  //   grpc::ClientContext context;
  //   auto stub = stubs_[addr].get();
  //   grpc::Status status = stub->Gossip(&context, request, &response);

  //   if (!status.ok()) {
  //     std::cerr << "Failed to gossip with " << addr << ": " << status.error_message() << std::endl;
  //   }
  // }
}

