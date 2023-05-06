# Minimum CMake required
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Project
project(dynamo-like-leveldb)

set(CMAKE_PREFIX_PATH "/users/yishen/out-s3;/users/yishen/out-grpc")

option(USE_S3_ADMIN "use s3 instead of our own local key value store" OFF)


# Protobuf
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

# Protobuf-compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)


# gRPC
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file
get_filename_component(hw_proto "gossip_node.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/gossip_node.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/gossip_node.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/gossip_node.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/gossip_node.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")



add_executable(admin_cli 
    admin_cli.cpp
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    admin_grpc.cpp
    state_machine.cpp)
target_link_libraries(admin_cli
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    stdc++fs)

add_executable(client_cli 
    client_cli.cpp
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    client_grpc.cpp
    state_machine.cpp)
target_link_libraries(client_cli
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    stdc++fs)

add_executable(consistent_hashing_ring_test 
    consistent_hashing_ring_test.cpp
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    consistent_hashing_ring.cpp)
target_link_libraries(consistent_hashing_ring_test
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    stdc++fs)

if(USE_S3_ADMIN)
    message("USE USE_S3_ADMIN hahah")
    add_definitions(-DUSE_S3_ADMIN)
    set(OPENSSL_CRYPTO_LIBRARY "/usr/lib/x86_64-linux-gnu/libcrypto.so")
    set(OPENSSL_SSL_LIBRARY "/usr/lib/x86_64-linux-gnu/libssl.so")
    find_package(OpenSSL REQUIRED)
    find_package(AWSSDK REQUIRED COMPONENTS s3)
    find_package(ZLIB REQUIRED)
    include_directories(${AWSSDK_INCLUDE_DIRS})
    link_libraries(${AWSSDK_LINK_LIBRARIES})
    find_package(Threads REQUIRED)
    find_path(LEVELDB_INCLUDE_DIR NAMES leveldb/db.h HINTS ../../leveldb/include)
    find_library(LEVELDB_LIBRARY NAMES leveldb HINTS ../../leveldb/build)
endif()
add_executable(gossip_node_test 
    gossip_node_test.cpp
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    gossip_node.cpp
    consistent_hashing_ring.cpp
    state_machine.cpp)
target_include_directories(gossip_node_test PRIVATE ${LEVELDB_INCLUDE_DIR})
target_link_libraries(gossip_node_test
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    stdc++fs
    ${LEVELDB_LIBRARY}
    Threads::Threads
    ${AWSSDK_LINK_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto)


add_executable(state_machine_test state_machine_test.cpp state_machine.cpp)

# foreach(_target
#     client_cli gossip_node_test consistent_hashing_ring_test)
#   target_link_libraries(${_target}
#     ${_REFLECTION}
#     ${_GRPC_GRPCPP}
#     ${_PROTOBUF_LIBPROTOBUF}
#     stdc++fs)
# endforeach()
