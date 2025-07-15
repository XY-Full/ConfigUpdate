
add_library(network_proto STATIC
    TcpServer.cpp
)
target_include_directories(network_proto PUBLIC ${CMAKE_SOURCE_DIR}/network)
target_link_libraries(network_proto PUBLIC pthread common)