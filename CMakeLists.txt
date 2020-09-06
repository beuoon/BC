project(YMC)

cmake_minimum_required(VERSION 3.18)

set(ROOT ..)
set(SRC_DIR ${ROOT}/Sources)
set(INC_DIR ${ROOT}/Includes)
set(RSC_DIR ${ROOT}/Resources)

include_directories(Includes)
include_directories(Libraries)
include_directories(Libraries/websocketpp)
include_directories(Libraries/asio/include)

set(SOURCE ${SRC_DIR}/YMC.cpp ${RSC_DIR}/YMC.rc)
add_executable(YMC WIN32 ${SOURCE})