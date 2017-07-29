cmake_minimum_required(VERSION 2.8.9)
project(concordpp-example)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/concordpp)

add_executable(runfile main.cpp)
target_link_libraries(runfile concordpp)

#Copy the token.txt file to the build directory.
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/token.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
