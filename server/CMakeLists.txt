
cmake_minimum_required(VERSION 3.10)
project(ConfigUpProtobuf)
set(CMAKE_CXX_STANDARD 17)

# Protobuf 路径
set(PROTOBUF_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../third/protobuf/include)
set(PROTOBUF_LIB_DIR     ${CMAKE_SOURCE_DIR}/../third/protobuf/lib)
link_directories(${PROTOBUF_LIB_DIR})
include_directories(${PROTOBUF_INCLUDE_DIR})


# 各子模块
add_subdirectory(../public proto)
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