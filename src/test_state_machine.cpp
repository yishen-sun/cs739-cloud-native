#include <iostream>
#include <cassert>
#include <vector>
#include "state_machine.h"

using namespace std;

int main() {
  StateMachine sm("test_storage.txt");

  // Test the put method
  sm.put("foo", "bar", {{"server1", 1}, {"server2", 2}});
  assert(sm.get_result("foo") == "bar");
  vector<pair<string, uint64_t>> result = sm.get_server_info("foo");

  for (int i = 0; i < result.size(); i++) {
    cout << result[i].first << " " << result[i].second << endl;
  }
  
  result = {{"server1", 1}, {"server2", 2}};
  for (int i = 0; i < result.size(); i++) {
    cout << result[i].first << " " << result[i].second << endl;
  }
 

  // Test the remove method
  sm.remove("foo", {{"server1", 1}});
  cout << sm.get_result("foo") << endl;

  return 0;
}