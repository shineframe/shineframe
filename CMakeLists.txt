PROJECT(shineframe_examples)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}/include
)

IF(NOT WIN32)
 SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall")

 LINK_DIRECTORIES(/usr/lib)
 LINK_DIRECTORIES(/usr/local/lib)
 LINK_DIRECTORIES(/usr/local/mysql/lib)
 LINK_DIRECTORIES(../bin) 
 LINK_LIBRARIES(dl)
 LINK_LIBRARIES(pthread)
 LINK_LIBRARIES(mysqlclient_r)
ENDIF()

ADD_EXECUTABLE(string examples/string.cpp)

ADD_EXECUTABLE(serial_model examples/serial_model.cpp)

ADD_EXECUTABLE(json_base examples/json_base.cpp)

ADD_EXECUTABLE(json_model examples/json_model.cpp)

ADD_EXECUTABLE(redis_sync examples/redis_sync.cpp)

ADD_EXECUTABLE(http_base_server examples/http_base_server.cpp)

ADD_EXECUTABLE(http_base_server_multi_thread examples/http_base_server_multi_thread.cpp)

ADD_EXECUTABLE(http_base_client examples/http_base_client.cpp)

ADD_EXECUTABLE(proactor_echo_server examples/proactor_echo_server.cpp)

ADD_EXECUTABLE(proactor_echo_client examples/proactor_echo_client.cpp)

ADD_EXECUTABLE(mysql examples/mysql.cpp)

ADD_EXECUTABLE(sqlite examples/sqlite.cpp include/sqlite/sqlite.hpp include/sqlite/sqlite/sqlite3.c)

ADD_EXECUTABLE(websocket_echo_server examples/websocket_echo_server.cpp)

ADD_EXECUTABLE(zeromq examples/zeromq.cpp)

ADD_EXECUTABLE(filesystem examples/filesystem.cpp)

ADD_EXECUTABLE(rpc_client examples/rpc_client.cpp)

ADD_EXECUTABLE(rpc_server examples/rpc_server.cpp)

ADD_EXECUTABLE(raft_nosql_example examples/raft_nosql_example.cpp)

ADD_EXECUTABLE(concurrent_queue examples/concurrent_queue.cpp)
