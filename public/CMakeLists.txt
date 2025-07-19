file(GLOB PROTO_CC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/proto_files/*.cc"
)

add_library(proto STATIC
    ${PROTO_CC_FILES}
)

target_include_directories(proto 
                            PUBLIC 
                            ${CMAKE_SOURCE_DIR}/../third/protobuf/include
                            ${CMAKE_SOURCE_DIR}/proto_files
                            )
                            
target_include_directories(proto PUBLIC protobuf)