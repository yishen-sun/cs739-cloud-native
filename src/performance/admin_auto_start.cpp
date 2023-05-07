#include <sstream>

#include "../admin_grpc.h"

using namespace std;

unordered_set<string> get_nodes_config(string nodes_config_) {
    unordered_set<string> server_addrs;
    std::ifstream file(nodes_config_);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            server_addrs.emplace(line);
        }
    }
    return server_addrs;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "you must provide two arguments: config_path state_machine_name" << std::endl;
        std::cout << "Usage: ./admin_cli server_config_for_admin.txt admin_sm" << std::endl;
        return 0;
    }
    string config_path = argv[1];
    string admin_sm = argv[2];
    Admin ad(config_path, admin_sm);

   
    std::string command("JoinNetwork"); 
    
    unordered_set<string> server_addrs = get_nodes_config(config_path);
    
    for (const auto& server_addr : server_addrs) {
        if (ad.Cmd(server_addr, command)) {
            std::cout << command << " successful." << std::endl;
        } else {
            std::cerr << command << " failed." << std::endl;
            return 0;
        }
    }
    return 0;
}