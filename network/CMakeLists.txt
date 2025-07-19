
file(GLOB NETWORK_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(network_proto STATIC
    ${NETWORK_SRC}
)

target_include_directories(network_proto PUBLIC ${CMAKE_SOURCE_DIR}/network)
target_link_libraries(network_proto PUBLIC pthread common)