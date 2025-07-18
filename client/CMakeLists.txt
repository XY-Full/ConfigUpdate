
cmake_minimum_required(VERSION 3.10)
project(ConfigUpProtobuf)
set(CMAKE_CXX_STANDARD 17)

# Protobuf 路径
set(PROTOBUF_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../protobuf/include)
set(PROTOBUF_LIB_DIR     ${CMAKE_SOURCE_DIR}/../protobuf/lib)
link_directories(${PROTOBUF_LIB_DIR})
include_directories(${PROTOBUF_INCLUDE_DIR})

# 添加公共头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}/../protobuf/include
    ${CMAKE_SOURCE_DIR}/../protobuf/proto_files
)

# 可执行文件
add_executable(client main.cpp config_update.pb.cc NetPack.cpp base.pb.cc base.pb.cc)
target_link_libraries(client
    PRIVATE protobuf pthread
)
