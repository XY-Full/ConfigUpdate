add_library(logic_modules STATIC
    ConfigUpdate.cpp
)

target_include_directories(logic_modules 
                            PUBLIC 
                            ${CMAKE_SOURCE_DIR}/logic
                            ${CMAKE_SOURCE_DIR}/busd
                            ${CMAKE_SOURCE_DIR}/protobuf/include
                            ${CMAKE_SOURCE_DIR}/protobuf/proto_files
                            )
                            
target_link_libraries(logic_modules PUBLIC common busd_core protobuf)