#include "gossip_node.h"

GossipNode::GossipNode(const std::string &node_id, int num_virtual_nodes, const std::string &server_address, const std::string& config_path)
    : node_id_(node_id), server_address_(server_address), ring_(node_id, num_virtual_nodes), state_machine_(node_id + "storage.txt", config_path) {
  // srand(static_cast<unsigned int>(time(nullptr)));
  // channel_ = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  // stub_ = gossipnode::GossipNodeService::NewStub(channel_);
  std::vector<std::string> res = read_server_config_update_stubs_();
  if (FIXED_CONFIG_TEST == true) {
    for (const auto& other_node : state_machine_.get_nodes_config()) {
      ring_.addNode(other_node);
    }
    ring_.printAllVirtualNode();
  } else {
    state_machine_.write_nodes_config(res);
  }
}


std::vector<std::string> GossipNode::read_server_config_update_stubs_() {
    // file format:
    // <name>/<addr> e.g. A/0.0.0.0:50001
    // TODO: link to S3
    std::vector<std::string> server_config = state_machine_.get_nodes_config();
    // std::ifstream infile(config_path);
    // std::string line;
    // while (std::getline(infile, line)) {
    //     size_t pos = line.find('/');
    //     if (pos != std::string::npos) {
    //         std::string key = line.substr(0, pos);
    //         std::string value = line.substr(pos + 1, line.size() - pos - 1);
    //         server_config[key] = value;
    //     }
    // }
    stubs_.clear();
    for (const auto& addr : server_config) {
        if (addr != node_id_) {
            stubs_[addr] =
                gossipnode::GossipNodeService::NewStub(grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()));
        }
    }
    server_config.push_back(node_id_);
    return server_config;
}

std::vector<std::string> GossipNode::getTransferKey(const std::string &node_id, int num_replicas) {
  ConsistentHashingRing tmp_ring(ring_);
  tmp_ring.addNode(node_id);
  std::vector<std::string> all_keys = state_machine_.get_all_keys();
  std::vector<std::string> filtered_keys;
  for (auto keys : all_keys) {
    if (tmp_ring.checkDataBelonging(keys, num_replicas) == false) filtered_keys.push_back(keys);
  }
  return filtered_keys;
}

bool GossipNode::joinNetwork() {
  bool flag = true;
  for (auto i : stubs_) {
    grpc::ClientContext context;
    gossipnode::JoinRequest request;
    gossipnode::JoinResponse response;
    request.set_node_id(node_id_);
    grpc::Status status = i.second->JoinNetwork(&context, request, &response);
    if (!status.ok()) {
      std::cerr << "joinNetwork RPC failed: " << i.first << " " << status.error_message() << std::endl;
      flag = false;
    } else {
      if (!response.success()) std::cout << "JoinNetwork response from node " << i.first << ": not success " << std::endl;
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
  std::vector<std::string> keys = getTransferKey(request->node_id(), RPELICA_N);
  response->set_success(true);
  for (auto key : keys) {
    int res = peerPut(request->node_id(), key, state_machine_.get_value(key), state_machine_.get_version(key));
    if (res > 0) {
      response->set_success(false);
    }
    if (res == 0) {
      node_transfer_keys_[request->node_id()].push_back(key);
    }
  }
  return grpc::Status::OK;
}

bool GossipNode::updateRing() {
  // Add myself to the ring
  ring_.addNode(node_id_);

  bool flag = true;
  for (auto i : stubs_) {
    grpc::ClientContext context;
    gossipnode::UpdateRingRequest request;
    gossipnode::UpdateRingResponse response;
    request.set_node_id(node_id_);
    grpc::Status status = i.second->UpdateRing(&context, request, &response);
    if (!status.ok()) {
      std::cerr << "UpdateRing RPC failed: " << i.first << " " << status.error_message() << std::endl;
      flag = false;
    } else {
      if (!response.success()) std::cout << "UpdateRing response from node " << i.first << ": not success " << std::endl;
      flag = false;
    }
  }
  
  if (flag) {
    // update membership list
    // members_heartbeat_list_[node_address] = std::chrono::high_resolution_clock::now();
    std::cout << "Successfully join the network" << std::endl;
  } else {
    std::cout << "Partial-successfully join the network" << std::endl;
  }
  return flag;
}

grpc::Status GossipNode::UpdateRing(grpc::ServerContext *context, const gossipnode::UpdateRingRequest *request, gossipnode::UpdateRingResponse *response) {
  // Add the joining node to the ring and update membership list
  ring_.addNode(request->node_id());
  //members_heartbeat_list_[request->node_id()] = std::chrono::high_resolution_clock::now();
  for(std::string key : node_transfer_keys_[request->node_id()]) {
    state_machine_.remove(key);
  }
  node_transfer_keys_.erase(request->node_id());
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
    cout << "trigger Client get" << endl;
    // read value from 3 replica, receive more than 2 acknowledgement before return to clients
    // data vector clock [[server_name, timestamp] ...] 
    string key = request->key();
    cout << "key: " << key << " hash: " << ring_.hashFunction(key) << endl;
    vector<string> replica_servers = ring_.getReplicasNodes(key, RPELICA_N);
    vector<pair<string, vector<pair<string, uint64_t>>>> results;
    // format: [D1, [[s1, 1], [s2, 2], [s3, 1]]]
    int success_cnt = 0;
    for (auto server : replica_servers) {
      if (server == server_address_) {
          results.push_back(make_pair(state_machine_.get_value(key), state_machine_.get_version(key)));
      } else {
        string value;
        vector<pair<string, uint64_t>> vector_clock;
        if (peerGet(server, key, value, vector_clock) == 0) {
          results.push_back(make_pair(value, vector_clock));
          success_cnt += 1;
        }
      }
    }
    results = state_machine_.get_latest_data(results);

    for (auto result : results) {
      gossipnode::Data *add_data = response->add_get_res_data();
      add_data->set_value(result.first);
      for (auto version_info : result.second) {
        gossipnode::ServerVersion* server_version = add_data->add_version_info();
        server_version->set_server(version_info.first);
        server_version->set_version(version_info.second);
        
      }
    }
    response->set_coordinator(replica_servers[0]);
    if (success_cnt >= R_COUNT) {
      return grpc::Status::OK;
    }
    return grpc::Status::CANCELLED;
}

grpc::Status GossipNode::ClientPut(grpc::ServerContext *context, const gossipnode::PutRequest *request, gossipnode::PutResponse *response) {
  cout << "trigger client put " << endl;
  // write value in 3 physical node, receive more than 2 acknowledgement before return to clients
  std::string key = request->key();
  std::string value = request->data().value();
  vector<pair<string, uint64_t>> versions;
  for (const auto& version_info : request->data().version_info()) {
    versions.push_back(make_pair(version_info.server(), version_info.version()));
  }

  cout << "PUT: key: " << key << " hash: " << ring_.hashFunction(key) << endl;

  std::vector<std::string> replica_servers = ring_.getReplicasNodes(key, RPELICA_N);
  bool check_coordinator = is_coordinator(replica_servers);
  response->set_coordinator(replica_servers[0]);
  if (check_coordinator == false) {
    response->set_ret(gossipnode::PutReturn::NOT_COORDINATOR);
    return grpc::Status::OK;
  }
  /* 
    remove this part because the client must read data before sending requests.
    It may be the new data without version info.
  */ 
  // if (request->data().version_info().size() == 0) {
  //   response->set_ret(gossipnode::PutReturn::NO_VERSION);
  //   return grpc::Status::OK;
  // }
  int success_cnt = 0;
  vector<pair<string, uint64_t>> new_version = state_machine_.update_version(versions, node_id_);
  for (const auto& peer_server : replica_servers) {
    if (peer_server == node_id_) continue;
    if (peerPut(peer_server, key, value, new_version) == 0) success_cnt++;
  }

  state_machine_.put(key, value, new_version);
  success_cnt++;
  if (success_cnt > W_COUNT) {
    response->set_ret(gossipnode::PutReturn::OK);
  }  else {

    cout << "success count" << success_cnt << endl;
    response->set_ret(gossipnode::PutReturn::FAILED);
  }
  return grpc::Status::OK;
}

grpc::Status GossipNode::PeerPut(grpc::ServerContext *context, const gossipnode::PeerPutRequest *request, gossipnode::PeerPutResponse *response) {
  cout << "Received Peerput " << endl;
  // put result into rings
  string key = request->key();
  string value = request->data().value();
  vector<pair<string, uint64_t>> new_version;
  for (const auto& version_pair : request->data().version_info()) {
    new_version.push_back(make_pair(version_pair.server(), version_pair.version()));
  }
  vector<pair<string, uint64_t>> old_version = state_machine_.get_version(key);
  response->set_success(1);
  cout << "inside peer put ------" << endl;
  print_version(new_version);
  cout << "old version: ----" << endl;
  print_version(old_version);
  
  if (state_machine_.check_conflict_version(new_version, old_version) == 1) {
    // only apply unconflict data
    state_machine_.put(key, value, new_version);
    response->set_success(0);
    cout << "inside peer put: applied log: " << key << " " << value << endl;
  }
  return grpc::Status::OK;
}

grpc::Status GossipNode::PeerGet(grpc::ServerContext *context, const gossipnode::PeerGetRequest *request, gossipnode::PeerGetResponse *response) {
  cout << "Received PeerGet" << endl;
  // get result, put result into request results.
  string key = request->key();
  string value = state_machine_.get_value(key);
  std::vector<std::pair<std::string, uint64_t>> version_infos = state_machine_.get_version(key);

  response->mutable_data()->set_value(value);
  for (const auto& version_info : version_infos) {
    gossipnode::ServerVersion* info = response->mutable_data()->add_version_info();
    info->set_server(version_info.first);
    info->set_version(version_info.second);
  }
  return grpc::Status::OK;
}

int GossipNode::peerPut(const string peer_server, const string key, const string& value, const vector<pair<string, uint64_t>>& vector_clock) {
    cout << "Send peer put" << endl;
    auto stub = stubs_[peer_server];
    grpc::ClientContext context;
    gossipnode::PeerPutRequest request;
    gossipnode::PeerPutResponse response;

    request.set_key(key);
    auto& data = *(request.mutable_data());
    data.set_value(value);
    for (const auto& version_pair : vector_clock) {
      auto& version_info = *(data.add_version_info());
      version_info.set_server(version_pair.first);
      version_info.set_version(version_pair.second);
    }
    
    grpc::Status status = stub->PeerPut(&context, request, &response);
    if (status.ok() && response.success() == 0) return 0;
    return 1;
};

int GossipNode::peerGet(const string peer_server, const string key, string& value, vector<pair<string, uint64_t>>& vector_clock) {
    cout << "sent peerget to:" << peer_server << endl;
    auto stub = stubs_[peer_server];
    grpc::ClientContext context;
    gossipnode::PeerGetRequest request;
    gossipnode::PeerGetResponse response;

    request.set_key(key);
    grpc::Status status = stub->PeerGet(&context, request, &response);
    if (!status.ok()) {
      cerr << "peerGet RPC failed: " << status.error_message() << endl;
      // Handle error
      return 1;
    }

    gossipnode::Data data = response.data();
    value = data.value();
    cout << "Value: " << value << endl;
    
    for (const auto& server_version : data.version_info()) {
      string server = server_version.server();
      uint64_t version = server_version.version();
      vector_clock.push_back(std::make_pair(server, version));
      cout << "  Server: " << server << ", Version: " << version << endl;
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

bool GossipNode::is_coordinator(vector<string>& quorum_member) {
  for (const auto& cur_server : quorum_member) {
    if (cur_server == node_id_) return true;
  }
  return false;
}

//TODO: shutdown after complete
// int main() {
//   return 0;
// }

void GossipNode::print_version(vector<pair<string, uint64_t>> versions) {
  for (const auto& v : versions) {
    cout << "Server: " << v.first << " Version: " << v.second << endl;
  }
}