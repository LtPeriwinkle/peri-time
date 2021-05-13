cmake_minimum_required (VERSION 3.6)
project(tzbot)

find_package(SQLite3 REQUIRED)
find_package(date REQUIRED)
find_package(Threads REQUIRED)

add_executable(tzbot src/main.cpp src/tz_client.cpp src/embed_templates.cpp include/tz_client.hpp include/embed_templates.hpp include/database.hpp)
target_include_directories(tzbot PUBLIC include)
add_subdirectory(deps/sleepy-discord)
set(USE_SYSTEM_TZ_DB ON)
set(BUILD_TZ_LIB ON)
target_link_libraries(tzbot sleepy-discord date::date-tz SQLite::SQLite3)
