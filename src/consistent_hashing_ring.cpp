#include "consistent_hashing_ring.h"


ConsistentHashingRing::ConsistentHashingRing(int num_virtual_nodes)
    : num_virtual_nodes_(num_virtual_nodes) {}

size_t ConsistentHashingRing::hashFunction(const std::string& key) {
    // output range [0, 2^64 - 1] = [0, 18,446,744,073,709,551,615].
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

std::vector<std::string> ConsistentHashingRing::getReplicasNodes(const std::string& key, int num_replicas) {
    std::vector<std::string> nodes_vec;

    if (ring_.empty()) {
        return nodes_vec;
    }

    std::unordered_set<std::string> nodes_set;
    size_t hash = hashFunction(key);
    size_t i = hash;
    while (nodes_vec.size() < num_replicas) {
        auto it = ring_.lower_bound(i);
        if (it == ring_.end()) {
            it = ring_.begin();
        }
        if (nodes_set.count(it->second) == 0) {
            nodes_set.emplace(it->second);
            nodes_vec.push_back(it->second);
        }
        i = (it->first)++;
    }
    
    return nodes_vec;
}
