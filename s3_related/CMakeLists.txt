cmake_minimum_required(VERSION 3.10)

project(leveldb_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_PREFIX_PATH "/users/yishen/out-s3;/users/yishen/out-grpc")

# Find OpenSSL package
set(OPENSSL_CRYPTO_LIBRARY "/usr/lib/x86_64-linux-gnu/libcrypto.so")
set(OPENSSL_SSL_LIBRARY "/usr/lib/x86_64-linux-gnu/libssl.so")

find_package(OpenSSL REQUIRED)

# Locate the AWS SDK for C++ package
find_package(AWSSDK REQUIRED COMPONENTS s3)
find_package(ZLIB REQUIRED)
# Include the AWS SDK for C++ headers
include_directories(${AWSSDK_INCLUDE_DIRS})

# Link against the AWS SDK for C++ libraries
link_libraries(${AWSSDK_LINK_LIBRARIES})

find_package(Threads REQUIRED)

find_path(LEVELDB_INCLUDE_DIR NAMES leveldb/db.h HINTS ../../leveldb/include)
find_library(LEVELDB_LIBRARY NAMES leveldb HINTS ../../leveldb/build)

add_executable(leveldb_example main.cpp)

target_include_directories(leveldb_example PRIVATE ${LEVELDB_INCLUDE_DIR})
target_link_libraries(leveldb_example PRIVATE
    ${LEVELDB_LIBRARY}
    Threads::Threads
    ${AWSSDK_LINK_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto)
