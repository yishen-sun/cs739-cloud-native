#include <iostream>
#include <cassert>
#include <vector>
#include "state_machine.h"

using namespace std;

int main() {
  StateMachine sm("test_storage.txt", "test_servers_addr.txt");
  cout << "Test case:  basic storage function ------------\n";
  // Test the put method
  sm.put("foo", "bar", {{"server1", 1}, {"server2", 2}});
  assert(sm.get_value("foo") == "bar");
  vector<pair<string, uint64_t>> result = sm.get_version("foo");

  for (int i = 0; i < result.size(); i++) {
    cout << result[i].first << " " << result[i].second << endl;
  }
  
  result = {{"server1", 1}, {"server2", 2}};
  for (int i = 0; i < result.size(); i++) {
    cout << result[i].first << " " << result[i].second << endl;
  }
 

  // Test the remove method
  sm.remove("foo"/*, {{"server1", 1}}*/);
  cout << sm.get_value("foo") << endl;

  cout << endl;
  // Test case for check fonflict version
  cout << "Test function:  check_conflict_version ------------\n";
  // Test case 1.a:  -> incoming > existing
  vector<pair<string, uint64_t>> incoming_empty{};
  vector<pair<string, uint64_t>> existing_empty{};
  int res = sm.check_conflict_version(incoming_empty, existing_empty);
  assert(res == 1);
  cout << "Test case 1.a passed\n";

  // Test case 1.b: One empty vector -> incoming is earlier
  vector<pair<string, uint64_t>> incoming_one_empty{};
  vector<pair<string, uint64_t>> existing_one{{"server_A", 1}};
  res = sm.check_conflict_version(incoming_one_empty, existing_one);
  assert(res == -1);
  cout << "Test case 1.b passed\n";

  // Test case 1.c: Non-overlapping vectors -> conflict
  vector<pair<string, uint64_t>> incoming_non_overlap{{"server_A", 2}, {"server_B", 1}};
  vector<pair<string, uint64_t>> existing_non_overlap{{"server_C", 1}, {"server_D", 2}};
  res = sm.check_conflict_version(incoming_non_overlap, existing_non_overlap);
  assert(res == 2);
  cout << "Test case 1.c passed\n";

  // Test case 1.d: Overlapping vectors, conflict
  vector<pair<string, uint64_t>> incoming_overlap{{"server_A", 2}, {"server_B", 1}};
  vector<pair<string, uint64_t>> existing_overlap{{"server_A", 2}, {"server_C", 1}};
  res = sm.check_conflict_version(incoming_overlap, existing_overlap);
  assert(res == 2);
  cout << "Test case 1.d passed\n";

  // Test case 1.e: Overlapping vectors, conflict
  vector<pair<string, uint64_t>> incoming_conflict{{"server_A", 1}, {"server_B", 2}};
  vector<pair<string, uint64_t>> existing_conflict{{"server_A", 2}, {"server_B", 1}};
  res = sm.check_conflict_version(incoming_conflict, existing_conflict);
  assert(res == 2);
  cout << "Test case 1.e passed\n";

  // Test case 1.f: Overlapping vectors, conflict
  vector<pair<string, uint64_t>> incoming_reverse{{"server_A", 2}, {"server_B", 1}};
  vector<pair<string, uint64_t>> existing_reverse{{"server_A", 1}, {"server_B", 2}};
  res = sm.check_conflict_version(incoming_reverse, existing_reverse);
  assert(res == 2);
  cout << "Test case 1.f passed\n";

  // Test case 1.g: Latest vectors -> 1
  vector<pair<string, uint64_t>> incoming_latest{{"server_A", 2}, {"server_B", 1}};
  vector<pair<string, uint64_t>> existing_earlier{{"server_A", 1}, {"server_B", 1}};
  res = sm.check_conflict_version(incoming_latest, existing_earlier);
  assert(res == 1);
  cout << "Test case 1.g passed\n";

  // Test case 1.g: Latest vectors -> 1
  vector<pair<string, uint64_t>> incoming_latest2{{"server_B", 2}, {"server_A", 1}};
  vector<pair<string, uint64_t>> existing_earlier2{{"server_A", 1}};
  res = sm.check_conflict_version(incoming_latest2, existing_earlier2);
  assert(res == 1);
  cout << "Test case 1.h passed\n";

  cout << "All test cases for check_conflict_version passed!" << endl << endl;


  cout << "Test function:  remove_duplicate_data ------------" << endl;
  // Define an input vector with duplicates
  vector<pair<string, vector<pair<string, uint64_t>>>> version_vectors = {
      {"value_1", {{"server_1", 1}, {"server_2", 2}}},
      {"value_1", {{"server_1", 1}, {"server_2", 2}}},
      {"value_3", {{"server_3", 3}, {"server_4", 4}}},
      {"value_3", {{"server_3", 3}, {"server_4", 4}}},
      {"value_1", {{"server_1", 1}, {"server_2", 2}}},
      {"value_5", {{"server_1", 1}, {"server_2", 2}}},
  };

  // Call the remove_duplicate_data function
  vector<pair<string, vector<pair<string, uint64_t>>>> unique_vectors =
      sm.remove_duplicate_data(version_vectors);

  // Define the expected output vector
  vector<pair<string, vector<pair<string, uint64_t>>>> expected_vectors = {
      {"value_1", {{"server_1", 1}, {"server_2", 2}}},
      {"value_3", {{"server_3", 3}, {"server_4", 4}}},
      {"value_5", {{"server_1", 1}, {"server_2", 2}}},
  };
  // Verify that the expected output vector matches the actual output vector
  assert(unique_vectors == expected_vectors);
  cout << "Test case 2.a passed\n";
  cout << "All test cases for remove_duplicate_data passed!" << endl << endl;
  

  cout << "Test function:  remove_unconflict_data ------------\n";
  // Test case 2.a
  // Define an input vector with conflicting data and with latest data version
  vector<pair<string, vector<pair<string, uint64_t>>>> original_vectors = {
      {"value_1", {{"server_1", 1}, {"server_2", 2}, {"server_3", 3}}},
      {"value_2", {{"server_1", 2}, {"server_3", 1}}},
      {"value_3", {{"server_1", 3}, {"server_2", 4}, {"server_3", 5}}},
  };

    
  vector<pair<string, vector<pair<string, uint64_t>>>> possible_conflict_data =
      sm.remove_unconflict_data(original_vectors);

  // Define the expected output vector
  expected_vectors = {
      {"value_2", {{"server_1", 2}, {"server_3", 1}}},
      {"value_3", {{"server_1", 3}, {"server_2", 4}, {"server_3", 5}}},
  };

  // Verify that the expected output vector matches the actual output vector
  assert(possible_conflict_data == expected_vectors);
  cout << "Test case 3.a passed\n";
  std::cout << "All test cases for remove_unconflict_data passed!" << endl << endl;



  cout << "Test function:  get_latest_data ------------" << endl;
  // Test case 3.a: Empty input
  vector<pair<string, vector<pair<string, uint64_t>>>> empty_input;
  vector<pair<string, vector<pair<string, uint64_t>>>> expected_output;
  assert(sm.get_latest_data(empty_input) == expected_output);
  cout << "Test case 4.a passed" << endl;

  // Test case 3.b: No duplicates or conflicts case
  vector<pair<string, vector<pair<string, uint64_t>>>> input = {
      {"value_1", {{"server_1", 10}, {"server_2", 20}}},
      {"value_2", {{"server_3", 30}, {"server_4", 40}}}
  };
  expected_output = input;
  assert(sm.get_latest_data(input) == expected_output);
  cout << "Test case 4.b passed" << endl;
  // Test case 3.c: Duplicates case
  input = {
      {"value_1", {{"server_1", 10}, {"server_2", 20}}},
      {"value_2", {{"server_3", 30}, {"server_4", 40}}},
      {"value_1", {{"server_1", 10}, {"server_2", 20}}}
  };
  expected_output = {
      {"value_1", {{"server_1", 10}, {"server_2", 20}}},
      {"value_2", {{"server_3", 30}, {"server_4", 40}}}
  };
  assert(sm.get_latest_data(input) == expected_output);
  cout << "Test case 4.c passed" << endl;
  // Test case 3.d: Conflicts case
  input = {
      {"value_1", {{"server_1", 10}, {"server_2", 20}}},
      {"value_2", {{"server_2", 30}, {"server_4", 40}}}
  };
  expected_output = {
      {"value_1", {{"server_1", 10}, {"server_2", 20}}},
      {"value_2", {{"server_2", 30}, {"server_4", 40}}}
  };
  assert(sm.get_latest_data(input) == expected_output);
  cout << "Test case 4.d passed" << endl;
  // Test case 3.e: Duplicates and conflicts
  input = {
      {"value_1", {{"server_1", 10}, {"server_2", 20}}},
      {"value_2", {{"server_3", 30}, {"server_4", 40}}},
      {"value_1", {{"server_1", 10}, {"server_2", 20}}},
      {"value_3", {{"server_3", 40}, {"server_4", 30}}},
  };
  expected_output = {
      {"value_1", {{"server_1", 10}, {"server_2", 20}}},
      {"value_2", {{"server_3", 30}, {"server_4", 40}}},
      {"value_3", {{"server_3", 40}, {"server_4", 30}}},
  };
  assert(sm.get_latest_data(input) == expected_output);
  cout << "Test case 4.e passed" << endl;
  std::cout << "All test cases for get_latest_data passed!" << std::endl;
  return 0;
}