project(BC)

cmake_minimum_required(VERSION 3.18)

set(ROOT ..)

file(GLOB SOURCES Sources/*.cpp)
file(GLOB HEADERS Includes/*)
file(GLOB RESOURCES Resources/*)

include_directories(Includes)
include_directories(Libraries)
include_directories(Libraries/websocketpp)
include_directories(Libraries/asio/asio/include)

add_executable(BC WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})