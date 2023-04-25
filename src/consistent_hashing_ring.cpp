#include "consistent_hashing_ring.h"
#include <functional>

ConsistentHashingRing::ConsistentHashingRing(int num_virtual_nodes)
    : num_virtual_nodes_(num_virtual_nodes) {}

size_t ConsistentHashingRing::hashFunction(const std::string& key) {
    //sha256
    return std::hash<std::string>{}(key);
}

void ConsistentHashingRing::addNode(const std::string& node_id) {
    for (int i = 0; i < num_virtual_nodes_; ++i) {
        std::string virtual_node_id = node_id + std::to_string(i);
        size_t hash = hashFunction(virtual_node_id);
        ring_[hash] = node_id;
    }
}

void ConsistentHashingRing::removeNode(const std::string& node_id) {
    for (int i = 0; i < num_virtual_nodes_; ++i) {
        std::string virtual_node_id = node_id + std::to_string(i);
        size_t hash = hashFunction(virtual_node_id);
        ring_.erase(hash);
    }
}

std::string ConsistentHashingRing::getNode(const std::string& key) {
    if (ring_.empty()) {
        return "";
    }

    size_t hash = hashFunction(key);
    auto it = ring_.lower_bound(hash);

    if (it == ring_.end()) {
        it = ring_.begin();
    }

    return it->second;
}

std::vector<std::string> ConsistentHashingRing::getNodesInRange(const std::string& start_key,
                                                                 const std::string& end_key) {
    // To be implemented, if needed
    return {};
}
