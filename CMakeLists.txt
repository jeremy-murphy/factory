cmake_minimum_required(VERSION 3.9)
project(multifactory)

find_package(Boost REQUIRED)

set(CMAKE_CXX_STANDARD 14)
add_executable(multifactory main.cpp)
target_include_directories(multifactory PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(multifactory ${Boost_LIBRARIES})

install(TARGETS multifactory RUNTIME DESTINATION bin)
