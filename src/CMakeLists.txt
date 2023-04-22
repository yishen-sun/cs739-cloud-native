cmake_minimum_required(VERSION 3.10)

project(leveldb_example)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Threads REQUIRED)

find_path(LEVELDB_INCLUDE_DIR NAMES leveldb/db.h HINTS ../leveldb/include)
find_library(LEVELDB_LIBRARY NAMES leveldb HINTS ../leveldb/build)

add_executable(leveldb_example main.cpp)

target_include_directories(leveldb_example PRIVATE ${LEVELDB_INCLUDE_DIR})
target_link_libraries(leveldb_example PRIVATE ${LEVELDB_LIBRARY} Threads::Threads)
