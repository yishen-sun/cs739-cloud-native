#include "gossip_node.h"

GossipNode::GossipNode(const std::string &node_id, int num_virtual_nodes, const std::string &server_address, const std::string& config_path)
    : node_id_(node_id), server_address_(server_address), ring_(node_id, num_virtual_nodes), state_machine_(node_id + "_storage.txt", config_path), alive_(false) {

  if (FIXED_CONFIG_TEST == true) {
    alive_ = true;
    std::unordered_set<std::string> res = read_server_config_update_stubs_();
    state_machine_.read_file();
    ring_.addNode(node_id_);
    // ring_.printAllVirtualNode();
  }
}

grpc::Status GossipNode::AdminCmd(grpc::ServerContext *context, const gossipnode::AdminCmdRequest *request, gossipnode::AdminCmdResponse *response) {
  response->set_success(false);
  if (request->cmd() == "JoinNetwork") {
    // acquire file write lock
    stubs_.clear();
    for (auto addr : request->alive_servers()) {
      std::cout << addr << std::endl;
      if (addr != node_id_) {
          ring_.addNode(addr);
          stubs_[addr] =
                gossipnode::GossipNodeService::NewStub(grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()));
        }
    }
    joinNetwork();
    alive_ = true;
    // release file write lock
    response->set_success(true);
  } else if (request->cmd() == "LeaveNetwork") {
    // acquire file write lock;
    alive_ = false;
    leaveNetwork();
    // release file write lock
    response->set_success(true);
  } else if (request->cmd() == "Ping") {
    response->set_success(alive_);
  } else {
    response->set_success(false);
  }
  return grpc::Status::OK;
}

std::unordered_set<std::string> GossipNode::read_server_config_update_stubs_() {
    // file format:
    // <name>/<addr> e.g. A/0.0.0.0:50001
    // TODO: link to S3
    std::unordered_set<std::string> server_config = state_machine_.get_nodes_config();
    stubs_.clear();
    for (const auto& addr : server_config) {
        if (addr != node_id_) {
          ring_.addNode(addr);
          stubs_[addr] =
                gossipnode::GossipNodeService::NewStub(grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()));
        }
    }
    server_config.emplace(node_id_);
    return server_config;
}

std::vector<std::string> GossipNode::getJoinTransferKey(const std::string &node_id, int num_replicas) {
  std::vector<std::string> all_keys = state_machine_.get_all_keys();
  std::vector<std::string> filtered_keys;
  for (auto keys : all_keys) {
    if (ring_.checkDataBelonging(keys, node_id, num_replicas) == false) filtered_keys.push_back(keys);
  }
  return filtered_keys;
}

std::vector<std::string> GossipNode::getLeaveTransferKey(ConsistentHashingRing& old_ring, const std::string &node_id, int num_replicas) {
  std::vector<std::string> all_keys = state_machine_.get_all_keys();
  std::vector<std::string> filtered_keys;
  
  for (auto keys : all_keys) {
    // find the key that is not on the old ring but on the new ring
    if (old_ring.checkDataBelonging(keys, node_id, num_replicas) == false && ring_.checkDataBelonging(keys, node_id, num_replicas) == true)
    filtered_keys.push_back(keys);
  }
  return filtered_keys;
}

bool GossipNode::joinNetwork() {
  std::cout << BLUE << "Send joinNetwork RPC" << RESET << std::endl;
  // Add myself to the ring
  ring_.addNode(node_id_);
  members_heartbeat_list_[node_id_] = std::chrono::high_resolution_clock::now();
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
    } else if (!response.success()) {
      std::cout << "JoinNetwork response from node " << i.first << ": not success " << std::endl;
      flag = false;
      
    }
  }
  ring_.printAllVirtualNode();
  return flag;
  // gossip(); // necessary?
}

grpc::Status GossipNode::JoinNetwork(grpc::ServerContext *context, const gossipnode::JoinRequest *request, gossipnode::JoinResponse *response) {
  std::cout << BLUE << "Received joinNetwork RPC" << RESET << std::endl;
  // Add the joining node to the ring and update stub
  ring_.addNode(request->node_id());
  members_heartbeat_list_[request->node_id()] = std::chrono::high_resolution_clock::now();
  stubs_[request->node_id()] = gossipnode::GossipNodeService::NewStub(grpc::CreateChannel(request->node_id(), grpc::InsecureChannelCredentials()));
  

  std::vector<std::string> keys = getJoinTransferKey(node_id_, REPLICA_N);
  response->set_success(true);
  for (auto key : keys) {
    int res = peerPut(request->node_id(), key, state_machine_.get_value(key), state_machine_.get_version(key));
    if (res > 0) {
      response->set_success(false);
    }
    if (res == 0) {
      state_machine_.remove(key);
      //node_transfer_keys_[request->node_id()].push_back(key);
    }
  }
  ring_.printAllVirtualNode();
  return grpc::Status::OK;
}

void GossipNode::leaveNetwork() {
  std::cout << BLUE << "Send leaveNetwork RPC" << RESET <<std::endl;
  ConsistentHashingRing old_ring(ring_);
  ring_.removeNode(node_id_);
  bool flag = true;
  for (auto i : stubs_) {
    grpc::ClientContext context;
    gossipnode::LeaveRequest request;
    gossipnode::LeaveResponse response;
    request.set_node_id(node_id_);
    grpc::Status status = i.second->LeaveNetwork(&context, request, &response);
    if (!status.ok()) {
      std::cerr << "leaveNetwork RPC failed: " << i.first << " " << status.error_message() << std::endl;
      flag = false;
    } else if (!response.success()) {
      std::cout << "leaveNetwork response from node " << i.first << ": not success " << std::endl;
      flag = false;
    } else {
      std::vector<std::string> keys = getLeaveTransferKey(old_ring, i.first, REPLICA_N);
      for (auto key : keys) {
        int k = 0;
        int res = peerPut(i.first, key, state_machine_.get_value(key), state_machine_.get_version(key));
        while ( res > 0 && k++ < 4) {
          res = peerPut(i.first, key, state_machine_.get_value(key), state_machine_.get_version(key));
          std::cout << "retry peerPut to " << i.first << " for " << k << " times" << std::endl;
        }
        if (res == 0) {
          state_machine_.remove(key);
          //node_transfer_keys_[request->node_id()].push_back(key);
        }
      }
    }
  }
  
  return;
}

grpc::Status GossipNode::LeaveNetwork(grpc::ServerContext *context, const gossipnode::LeaveRequest *request, gossipnode::LeaveResponse *response) {
  std::cout << "Received LeaveNetwork RPC" << std::endl;
  ring_.removeNode(request->node_id());
  stubs_.erase(request->node_id());
  members_heartbeat_list_.erase(request->node_id());
  response->set_success(true);
  return grpc::Status::OK;
}

int64_t time_point_to_ns(const std::chrono::time_point<std::chrono::high_resolution_clock>& tp) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
}

std::chrono::time_point<std::chrono::high_resolution_clock> ns_to_time_point(int64_t ns) {
  return std::chrono::high_resolution_clock::time_point(std::chrono::nanoseconds(ns));
}

void GossipNode::gossip() {
  cout << GREEN << "Send gossip" << RESET << endl;
  members_heartbeat_list_[node_id_] = std::chrono::high_resolution_clock::now();

  // TODO: Select random nodes from the membership list to gossip with
  std::vector<std::string> random_nodes;

  gossipnode::GossipRequest request;
  gossipnode::GossipResponse response;

  // Prepare the gossip request with the local membership list
  for (const auto& kv : members_heartbeat_list_) {
    gossipnode::KeyTimePair* kv_pair = request.add_member_list();
    kv_pair->set_key(kv.first);
    kv_pair->set_time_point_ns(time_point_to_ns(kv.second));
  }
    
  for (auto it : stubs_) {
    auto cur_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - members_heartbeat_list_[it.first]);
    if (duration_ms < std::chrono::milliseconds(CHECK_ALIVE_INTERVAL)) {
      grpc::ClientContext context;
      grpc::Status status = it.second->Gossip(&context, request, &response);

      if (!status.ok() || !response.success()) {
        std::cerr << "Failed to gossip with " << it.first << ": " << status.error_message() << std::endl;
      }
    }
  }
}

grpc::Status GossipNode::Gossip(grpc::ServerContext *context, const gossipnode::GossipRequest *request, gossipnode::GossipResponse *response) {
  cout << GREEN << "Received gossip " << RESET << endl;
  for (const auto& kv_pair : request->member_list()) {
    auto tp1 = ns_to_time_point(kv_pair.time_point_ns());
    auto tp2 = members_heartbeat_list_.count(kv_pair.key()) ? members_heartbeat_list_[kv_pair.key()] : std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::high_resolution_clock::duration::min());
    auto tp = tp1 > tp2 ? tp1 : tp2;
    members_heartbeat_list_[kv_pair.key()] = tp;
  }
  response->set_success(true);
  return grpc::Status::OK;
}

grpc::Status GossipNode::ClientGet(grpc::ServerContext *context, const gossipnode::GetRequest *request, gossipnode::GetResponse *response) {
    cout << CYAN << "Received client get" << RESET << endl;
    // read value from 3 replica, receive more than 2 acknowledgement before return to clients
    // data vector clock [[server_name, timestamp] ...] 
    string key = request->key();
    cout << "key: " << key << " hash: " << ring_.hashFunction(key) << endl;
    vector<string> replica_servers = ring_.getReplicasNodes(key, 2 * REPLICA_N);
    vector<pair<string, vector<pair<string, uint64_t>>>> results;
    // format: [D1, [[s1, 1], [s2, 2], [s3, 1]]]
    int success_cnt = 0, try_cnt = 0;
    for (auto& peer_server : replica_servers) {
      if (peer_server == node_id_) {
        try_cnt++;
        success_cnt++;
        results.push_back(make_pair(state_machine_.get_value(key), state_machine_.get_version(key)));
        if (try_cnt == REPLICA_N) break;
        continue;
      } 
      auto cur_time = std::chrono::high_resolution_clock::now();
      auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - members_heartbeat_list_[peer_server]);
      if (duration_ms < std::chrono::milliseconds(CHECK_ALIVE_INTERVAL)) {
        // only send to alive machine
        try_cnt++;
        string value;
        vector<pair<string, uint64_t>> vector_clock;
        if (peerGet(peer_server, key, value, vector_clock) == 0) {
          results.push_back(make_pair(value, vector_clock));
          success_cnt += 1;
          if (try_cnt == REPLICA_N) break;
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
    string coordinator_candidate = assigned_coordinator(replica_servers);
    response->set_coordinator(coordinator_candidate);
    if (success_cnt >= R_COUNT) {
      return grpc::Status::OK;
    }
    return grpc::Status::CANCELLED;
}

grpc::Status GossipNode::ClientPut(grpc::ServerContext *context, const gossipnode::PutRequest *request, gossipnode::PutResponse *response) {
  cout << CYAN << "Received client put " << RESET << endl;
  // write value in 3 physical node, receive more than 2 acknowledgement before return to clients
  std::string key = request->key();
  std::string value = request->data().value();
  vector<pair<string, uint64_t>> versions;
  for (const auto& version_info : request->data().version_info()) {
    versions.push_back(make_pair(version_info.server(), version_info.version()));
  }

  cout << "PUT: key: " << key << " hash: " << ring_.hashFunction(key) << endl;

  std::vector<std::string> replica_servers = ring_.getReplicasNodes(key, 2 * REPLICA_N);
  string coordinator_candidate = assigned_coordinator(replica_servers);
  bool check_coordinator = is_coordinator(replica_servers);
  response->set_coordinator(coordinator_candidate);
  if (check_coordinator == false) {
    response->set_ret(gossipnode::PutReturn::NOT_COORDINATOR);
    return grpc::Status::OK;
  }
  
  vector<pair<string, uint64_t>> new_version = state_machine_.update_version(versions, node_id_);

  int success_cnt = 0;
  int try_cnt = 0;
  for (auto peer_server : replica_servers) {
    if (peer_server == node_id_) {
      try_cnt++;
      success_cnt++;
      state_machine_.put(key, value, new_version);
      if (try_cnt == REPLICA_N) break;
      continue;
    }
    auto cur_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - members_heartbeat_list_[peer_server]);
    if (duration_ms < std::chrono::milliseconds(CHECK_ALIVE_INTERVAL)) {
      // only send to alive machine
      try_cnt++;
      vector<pair<string, uint64_t>> vector_clock;
      if (peerPut(peer_server, key, value, new_version) == 0) {
        success_cnt += 1;
        if (try_cnt == REPLICA_N) break;
      }
    }
  }


  if (success_cnt >= W_COUNT) {
    response->set_ret(gossipnode::PutReturn::OK);
  }  else {

    // cout << "success count" << success_cnt << endl;
    response->set_ret(gossipnode::PutReturn::FAILED);
  }
  return grpc::Status::OK;
}

grpc::Status GossipNode::PeerPut(grpc::ServerContext *context, const gossipnode::PeerPutRequest *request, gossipnode::PeerPutResponse *response) {
  cout << MAGENTA << "Received peer put " << RESET << endl;
  // put result into rings
  string key = request->key();
  string value = request->data().value();
  vector<pair<string, uint64_t>> new_version;
  for (const auto& version_pair : request->data().version_info()) {
    new_version.push_back(make_pair(version_pair.server(), version_pair.version()));
  }
  vector<pair<string, uint64_t>> old_version = state_machine_.get_version(key);
  response->set_success(1);
  // cout << "inside peer put ------" << endl;
  // print_version(new_version);
  // cout << "old version: ----" << endl;
  // print_version(old_version);
  // cout << "check conflict version: " << state_machine_.check_conflict_version(new_version, old_version) << endl;
  if (state_machine_.check_conflict_version(new_version, old_version) == 1) {
    // only apply unconflict data
    state_machine_.put(key, value, new_version);
    response->set_success(0);
    // cout << "inside peer put: applied log: " << key << " " << value << endl;
  }
  return grpc::Status::OK;
}

grpc::Status GossipNode::PeerGet(grpc::ServerContext *context, const gossipnode::PeerGetRequest *request, gossipnode::PeerGetResponse *response) {
  cout << MAGENTA << "Received peer get" << RESET << endl;
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
    cout << MAGENTA << "Send peer put to: " << peer_server << RESET << endl;
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
    cout << MAGENTA << "Send peer get to: " << peer_server << RESET << endl;
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
    // cout << "Value: " << value << endl;
    
    for (const auto& server_version : data.version_info()) {
      string server = server_version.server();
      uint64_t version = server_version.version();
      vector_clock.push_back(std::make_pair(server, version));
      // cout << "  Server: " << server << ", Version: " << version << endl;
    }
  
    return 0;
}

bool GossipNode::is_coordinator(vector<string>& quorum_member) {
  for (const auto& cur_server : quorum_member) {
    if (cur_server == node_id_) return true;
  }
  return false;
}

void GossipNode::print_version(vector<pair<string, uint64_t>> versions) {
  for (const auto& v : versions) {
    cout << "Server: " << v.first << " Version: " << v.second << endl;
  }
}

string GossipNode::assigned_coordinator(vector<string>& quorum_member) {
  for (const auto& candidate : quorum_member) {
    auto cur_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - members_heartbeat_list_[candidate]);
    if (duration_ms < std::chrono::milliseconds(CHECK_ALIVE_INTERVAL)) { 
      return candidate;
    }
  }
  return "";
}