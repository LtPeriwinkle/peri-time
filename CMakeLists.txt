cmake_minimum_required (VERSION 3.6)
project(tzbot)

set(USE_SYSTEM_TZ_DB ON)
set(BUILD_TZ_LIB ON)
add_subdirectory(deps/date)
find_package(SQLite3 REQUIRED)

add_executable(tzbot src/main.cpp src/tz_client.cpp src/embed_templates.cpp include/tz_client.hpp include/embed_templates.hpp include/database.hpp)
target_include_directories(tzbot PUBLIC include)
add_subdirectory(deps/sleepy-discord)
target_link_libraries(tzbot PUBLIC sleepy-discord date-tz SQLite::SQLite3)
