cmake_minimum_required (VERSION 3.6)
project(tzbot)

add_executable(tzbot src/main.cpp)
add_subdirectory(deps/sleepy-discord)
target_link_libraries(tzbot sleepy-discord)
