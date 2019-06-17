cmake_minimum_required(VERSION 3.0)
project(expected)

# Includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/expected/include
)

# Flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra $ENV{STDLIB}")

# Tests
file(GLOB SOURCES "tests/*.cpp")
add_executable(tests ${SOURCES})
target_include_directories(tests PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/catch2
    ${CMAKE_CURRENT_SOURCE_DIR}/test
)
