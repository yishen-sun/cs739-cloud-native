cmake_minimum_required(VERSION 3.10)

project(leveldb_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Locate the AWS SDK for C++ package
find_package(AWSSDK REQUIRED COMPONENTS s3)
find_package(ZLIB REQUIRED)
# Include the AWS SDK for C++ headers
include_directories(${AWSSDK_INCLUDE_DIRS})

# Link against the AWS SDK for C++ libraries
link_libraries(${AWSSDK_LINK_LIBRARIES})

# Add the executable target and source files
# add_executable(s3_file_modifier tmp.cpp)

# Link the executable target against the AWS SDK for C++ libraries
# target_link_libraries(leveldb_example ${AWSSDK_LINK_LIBRARIES})


find_package(Threads REQUIRED)

find_path(LEVELDB_INCLUDE_DIR NAMES leveldb/db.h HINTS ../../leveldb/include)
find_library(LEVELDB_LIBRARY NAMES leveldb HINTS ../../leveldb/build)

add_executable(leveldb_example main.cpp)

target_include_directories(leveldb_example PRIVATE ${LEVELDB_INCLUDE_DIR})
target_link_libraries(leveldb_example PRIVATE ${LEVELDB_LIBRARY} Threads::Threads ${AWSSDK_LINK_LIBRARIES})