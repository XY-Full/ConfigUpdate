
cmake_minimum_required(VERSION 3.10)
project(ConfigUpProtobuf)
set(CMAKE_CXX_STANDARD 17)

# Protobuf 路径
set(PROTOBUF_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/protobuf/include)
set(PROTOBUF_LIB_DIR     ${CMAKE_SOURCE_DIR}/protobuf/lib)
link_directories(${PROTOBUF_LIB_DIR})
include_directories(${PROTOBUF_INCLUDE_DIR})

# 添加公共头文件路径
include_directories(
    ${CMAKE_SOURCE_DIR}/busd
    ${CMAKE_SOURCE_DIR}/logic
    ${CMAKE_SOURCE_DIR}/network
    ${CMAKE_SOURCE_DIR}/protobuf/include
    ${CMAKE_SOURCE_DIR}/protobuf/proto_files
    ${CMAKE_SOURCE_DIR}/common
)

# 各子模块
add_subdirectory(protobuf)
add_subdirectory(network)
add_subdirectory(busd)
add_subdirectory(logic)
add_subdirectory(common)

# 可执行文件
add_executable(server main.cpp)
target_link_libraries(server
    PRIVATE network_proto
            common
            busd_core
            logic_modules
            proto
)