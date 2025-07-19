file(GLOB BUSD_CORE_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(busd_core STATIC
    ${BUSD_CORE_SRC}
)

target_include_directories(busd_core 
                            PUBLIC 
                            ${CMAKE_SOURCE_DIR}/busd
                            ${CMAKE_SOURCE_DIR}/network
                            ${CMAKE_SOURCE_DIR}/protobuf/include
                            ${CMAKE_SOURCE_DIR}/../public/proto_files
                            ${CMAKE_SOURCE_DIR}/common
                            )
                            
target_include_directories(busd_core PUBLIC common network)