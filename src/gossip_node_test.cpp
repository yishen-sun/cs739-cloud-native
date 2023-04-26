#include "gossip_node.h"
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <node_id> <num_virtual_nodes> <server_address> [<join_address>]" << std::endl;
    return 1;
  }

  std::string node_id = argv[1];
  int num_virtual_nodes = std::stoi(argv[2]);
  std::string server_address = argv[3];

  GossipNode node(node_id, num_virtual_nodes, server_address);

  // Join network through another node, if provided
  if (argc == 5) {
    std::string join_address = argv[4];
    node.joinNetwork(join_address);
  }

  // Start gRPC server
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&node);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
  return 0;
}
