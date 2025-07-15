
add_library(busd_core STATIC
    Busd.h
    ModuleManager.cpp
)

target_include_directories(busd_core 
                            PUBLIC 
                            ${CMAKE_SOURCE_DIR}/logic
                            ${CMAKE_SOURCE_DIR}/busd
                            ${CMAKE_SOURCE_DIR}/protobuf/include
                            ${CMAKE_SOURCE_DIR}/protobuf/proto_files
                            ${CMAKE_SOURCE_DIR}/common
                            )
                            
target_include_directories(busd_core PUBLIC ${CMAKE_SOURCE_DIR}/busd common)