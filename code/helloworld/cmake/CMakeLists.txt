cmake_minimum_required (VERSION 3.1)

set(PROJECT_NAME helloworld)
project(${PROJECT_NAME})

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_BUILD_TYPE Debug)

add_executable(${PROJECT_NAME} src/main.cpp)
link_directories("../../../libuv/.libs")
target_link_libraries(${PROJECT_NAME} libuv.a)

