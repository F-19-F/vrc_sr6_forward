cmake_minimum_required(VERSION 3.10)
project(udp_ws_bridge)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)

include_directories(asio/asio/include)
include_directories(websocketpp)
include_directories(json/include)

add_executable(udp_ws_bridge main.cpp)

target_link_libraries(udp_ws_bridge Threads::Threads)
