project(PackageTest CXX)
cmake_minimum_required(VERSION 2.8.12)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
include(res/Catch)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fstrict-aliasing -Werror")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()
add_executable(skizzay_fsm_tests
   test_main.cpp
   integration/actionable_transitions.t.cpp
   integration/guarded_transitions.t.cpp
   integration/simple_transitions.t.cpp
   integration/traffic_light.t.cpp
   unit/dispatcher.t.cpp
   unit/state_container.t.cpp
   unit/traits.t.cpp
   )
catch_discover_tests(skizzay_fsm_tests)
