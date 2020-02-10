include_directories(${PROJECT_SOURCE_DIR}/thirdparty/tinyxml)
include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib/tinyxml)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/cmuduo-rpc)

aux_source_directory(. SRC_LIST)

add_library(cmuduo-rpc ${SRC_LIST})
target_link_libraries(cmuduo-rpc muduo_net muduo_base protobuf zookeeper_mt tinyxml pthread)
