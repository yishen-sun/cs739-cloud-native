#ifndef CONSISTENT_HASHING_RING_H
#define CONSISTENT_HASHING_RING_H

#include <map>
#include <string>
#include <vector>
#include <unordered_set>
#include <functional>

class ConsistentHashingRing {
public:
    ConsistentHashingRing(int num_virtual_nodes);

    void addNode(const std::string& node_id);
    void removeNode(const std::string& node_id);
    std::string getNode(const std::string& key);
    std::vector<std::string> getReplicasNodes(const std::string& key, int num_replicas);

private:
    int num_virtual_nodes_;
    std::map<size_t, std::string> ring_;

    size_t hashFunction(const std::string& key);
};

#endif // CONSISTENT_HASHING_RING_H
