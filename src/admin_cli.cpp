#include <sstream>

#include "./admin_grpc.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "you must provide two arguments: config_path state_machine_name" << std::endl;
        std::cout << "Usage: ./admin_cli server_config_for_admin.txt admin_sm" << std::endl;
        return 0;
    }
    string config_path = argv[1];
    string admin_sm = argv[2];
    Admin ad(config_path, admin_sm);

    std::string input;
    std::string cmd, addr, result;
    while (true) {
        std::cout << "Enter command Ping/JoinNetwork/LeaveNetwork with addr or" <<std::endl;
        std::cout << "Enter command exit/print" <<std::endl;
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string command, addr;
        iss >> command;
        if (command == "Ping" || command == "JoinNetwork" || command == "LeaveNetwork") {
            iss >> addr;
            if (ad.Cmd(addr, command)) {
                std::cout << command << " successful." << std::endl;
            } else {
                std::cout << command << " failed." << std::endl;
            }
        } else if (command == "exit") {
            break;
        } else if (command == "print") {
            ad.printAllNodeStatus();
        }
    }
    return 0;
}