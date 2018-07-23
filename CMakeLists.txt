project(fsm CXX)
cmake_minimum_required(VERSION 3.11.2)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fstrict-aliasing -Werror")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories("${PROJECT_SOURCE_DIR}/src")
add_subdirectory("src")

enable_testing()
add_subdirectory("test")
