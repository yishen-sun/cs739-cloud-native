#include <cassert>
#include <iostream>
#include <string>
#include "leveldb/wrapper_db.h"

int main() {
  leveldb::LevelDBWrapper* db = new leveldb::LevelDBWrapper();
  leveldb::Options options;
  options.create_if_missing = true;

  // Open or create the LevelDB database
  leveldb::Status status = db->Open(options, "testdb");
  assert(status.ok());

  // Write a key-value pair to the database
  std::string key = "example_key";
  std::string value = "example_value";
  status = db->Put(leveldb::WriteOptions(), key, value);
  assert(status.ok());

  // Read the value associated with the key
  std::string read_value;
  status = db->Get(leveldb::ReadOptions(), key, &read_value);
  assert(status.ok());
  std::cout << "Read value: " << read_value << std::endl;

  // Delete the key-value pair from the database
  status = db->Delete(leveldb::WriteOptions(), key);
  assert(status.ok());

  // Close the database
  delete db;

  return 0;
}
