#include "gossip_node.h"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <node_id> <num_virtual_nodes> <server_address> <config_path>" << std::endl;
    return 1;
  }

  std::string node_id = argv[1];
  int num_virtual_nodes = std::stoi(argv[2]);
  std::string server_address = argv[3];
  std::string config_path = argv[4];;

  GossipNode node(node_id, num_virtual_nodes, server_address, config_path);

  // Join network through another node, if provided
  // if (argc == 5) {
  //   std::string join_address = argv[4];
  //   node.joinNetwork(join_address);
  // }

  // Start gRPC server
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&node);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  while (true) {
    if (node.alive_) {
      auto cur_time = std::chrono::high_resolution_clock::now();
      auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - node.members_heartbeat_list_[node_id]);
      //std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (duration_ms > std::chrono::milliseconds(HEARTBEAT_INTERVAL)) node.gossip();
    }
  }
  server->Wait();
  return 0;
}
