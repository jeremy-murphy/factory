cmake_minimum_required(VERSION 3.6)
project(multifactory)

find_package(Boost REQUIRED)

set(CMAKE_CXX_STANDARD 11)
add_executable(multifactory main.cpp)
target_include_directories(multifactory PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(multifactory ${Boost_LIBRARIES})

install(TARGETS multifactory RUNTIME DESTINATION bin)
