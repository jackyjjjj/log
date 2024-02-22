cmake_minimum_required(VERSION 3.15)
project(log)

set(HOME /home/jyl/space/log)
file(GLOB SRC ${HOME}/*.cpp)

include_directories(${HOME}/*.h)
set(EXECUTABLE_OUTPUT_PATH ${HOME}/build)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall")

add_executable(log ${SRC})