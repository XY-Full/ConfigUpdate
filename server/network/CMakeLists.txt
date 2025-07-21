
file(GLOB NETWORK_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(network STATIC
    ${NETWORK_SRC}
)

# 添加头文件搜索路径
target_include_directories(network
    PUBLIC 
    ${CMAKE_SOURCE_DIR}/common
)

target_include_directories(network PUBLIC ${CMAKE_SOURCE_DIR}/network)
target_link_libraries(network PUBLIC pthread common)