add_library(zksync SHARED zk_sync.cxx)

target_link_libraries(zksync
    PUBLIC pthread 
    PUBLIC zookeeper_mt)

target_include_directories(zksync
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})