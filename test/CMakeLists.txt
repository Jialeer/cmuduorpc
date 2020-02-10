set(CLIENT_SRC ./client.cpp ./service.pb.cc)
set(SERVER_SRC ./service.cpp ./service.pb.cc)

include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib/cmuduo-rpc)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(testserver ${SERVER_SRC})
target_link_libraries(testserver cmuduo-rpc)
add_executable(testclient ${CLIENT_SRC})
target_link_libraries(testclient cmuduo-rpc)