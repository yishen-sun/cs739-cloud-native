#include "consistent_hashing_ring.h"

int main () {
    ConsistentHashingRing chr("a",3);
    chr.printAllVirtualNode();
    chr.addNode("a");
    chr.printAllVirtualNode();
    chr.addNode("b");
    chr.printAllVirtualNode();
    chr.removeNode("a");
    chr.printAllVirtualNode();
    std::cout << "k1 all replicas located on: " << chr.getNode("k1") << std::endl;
    chr.addNode("a");
    chr.printAllVirtualNode();
    chr.addNode("c");
    chr.printAllVirtualNode();
    chr.addNode("d");
    chr.printAllVirtualNode();

    std::vector<std::string> rep = chr.getReplicasNodes("k1", 3);
    std::cout << "k1 all replicas located on: ";
    for (auto s : rep) {
        std::cout << s << " ";
    }
    std::cout << std::endl;

    return 1;
}