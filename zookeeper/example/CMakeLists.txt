set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(c1 c_1.cxx)
add_executable(c2 c_2.cxx)
add_executable(c3 c_3.cxx)

target_link_libraries(c1
    PUBLIC zookeeper_mt
    PUBLIC pthread
    PUBLIC zksync)

target_link_libraries(c2
    PUBLIC zookeeper_mt
    PUBLIC pthread
    PUBLIC zksync)

target_link_libraries(c3
PUBLIC zookeeper_mt
PUBLIC pthread
PUBLIC zksync)