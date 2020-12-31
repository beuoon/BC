project(BC)

cmake_minimum_required(VERSION 3.18)

set(ROOT ..)
set(SRC_DIR ${ROOT}/Sources)
set(RSC_DIR ${ROOT}/Resources)

include_directories(Includes)
include_directories(Libraries)
include_directories(Libraries/websocketpp)
include_directories(Libraries/asio/asio/include)

set(SOURCE ${SRC_DIR}/BC.cpp ${RSC_DIR}/BC.rc)
add_executable(BC WIN32 ${SOURCE})