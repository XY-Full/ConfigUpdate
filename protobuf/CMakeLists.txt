file(GLOB PROTO_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/proto_files/*.cc")

add_library(proto ${PROTO_SRCS})

set(PROTOBUF_LIB_DIR     ${CMAKE_SOURCE_DIR}/protobuf/lib)

target_include_directories(proto
                            PUBLIC 
                            ${CMAKE_SOURCE_DIR}/protobuf/include
                            ${CMAKE_SOURCE_DIR}/protobuf/proto_files
                            )
                            
target_include_directories(proto PUBLIC protobuf)