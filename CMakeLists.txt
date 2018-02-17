cmake_minimum_required(VERSION 2.6)
project(multifactory)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS}) # yeah, yeah, I know

add_executable(multifactory main.cpp)

install(TARGETS multifactory RUNTIME DESTINATION bin)
