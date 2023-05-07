# Minimum CMake required
cmake_minimum_required(VERSION 3.5.1)
set(CMAKE_CXX_STANDARD 20)

# Project
project(dynamo-like-leveldb)



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

add_executable(gossip_node_test 
    gossip_node_test.cpp
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    gossip_node.cpp
    consistent_hashing_ring.cpp
    state_machine.cpp)
target_link_libraries(gossip_node_test
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    stdc++fs)

add_executable(state_machine_test state_machine_test.cpp state_machine.cpp)


add_executable(performance_test "./performance/throughput.cpp"
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    client_grpc.h
    client_grpc.cpp
    state_machine.cpp
    ./performance/throughput.h)
target_link_libraries(performance_test
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    stdc++fs)

add_executable(admin_auto_start "./performance/admin_auto_start.cpp"
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    admin_grpc.cpp
    state_machine.cpp)
target_link_libraries(admin_auto_start
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    stdc++fs)

# foreach(_target
#     client_cli gossip_node_test consistent_hashing_ring_test)
#   target_link_libraries(${_target}
#     ${_REFLECTION}
#     ${_GRPC_GRPCPP}
#     ${_PROTOBUF_LIBPROTOBUF}
#     stdc++fs)
# endforeach()
