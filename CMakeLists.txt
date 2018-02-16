cmake_minimum_required(VERSION 2.6)
project(multifactory)

add_executable(multifactory main.cpp)

install(TARGETS multifactory RUNTIME DESTINATION bin)
