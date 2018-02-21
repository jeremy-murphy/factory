cmake_minimum_required(VERSION 3.9)
project(multifactory)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS}) # yeah, yeah, I know

set(CMAKE_CXX_STANDARD 14)
add_executable(multifactory main.cpp)

install(TARGETS multifactory RUNTIME DESTINATION bin)
