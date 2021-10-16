find_package(Catch2 CONFIG REQUIRED)

add_executable(fsm_unit_tests)
target_link_libraries(fsm_unit_tests fsm Catch2::Catch2)
set_property(TARGET fsm_unit_tests PROPERTY CXX_STANDARD 20)
target_include_directories(fsm_unit_tests PRIVATE ${PROJECT_SOURCE_DIR}/src/main/cpp)
target_sources(fsm_unit_tests PRIVATE main.cpp
  skizzay/fsm/action_transition.t.cpp
  skizzay/fsm/ancestors.t.cpp
  # skizzay/fsm/flat_state_container.t.cpp
  skizzay/fsm/guarded_action_transition.t.cpp
  skizzay/fsm/guarded_transition.t.cpp
  # skizzay/fsm/hierarchical_state_container.t.cpp
  # skizzay/fsm/orthogonal_state_container.t.cpp
  skizzay/fsm/simple_transition.t.cpp
  skizzay/fsm/single_state_container.t.cpp
  skizzay/fsm/traits.t.cpp
  skizzay/fsm/type_list.t.cpp
)

add_executable(fsm_integration_tests)
target_link_libraries(fsm_integration_tests fsm Catch2::Catch2)
set_property(TARGET fsm_integration_tests PROPERTY CXX_STANDARD 20)
target_include_directories(fsm_integration_tests PRIVATE ${PROJECT_SOURCE_DIR}/src/main/cpp)
target_sources(fsm_integration_tests PRIVATE main.cpp
  #skizzay/fsm/integration/full_fsm.t.cpp
)



  include(CTest)
  include(Catch)
  catch_discover_tests(fsm_unit_tests)
  catch_discover_tests(fsm_integration_tests)
  enable_testing()


if(CMAKE_COMPILER_IS_GNUCXX)
  LIST(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/src/test/cmake")
  include(CodeCoverage)
  APPEND_COVERAGE_COMPILER_FLAGS()
  # set(COVERAGE_LCOV_EXCLUDES 'dir1/*' 'dir2/*') // this is optional if you want to exclude some directory from the report
  # SETUP_TARGET_FOR_COVERAGE_LCOV(NAME foo_coverage
  #                                EXECUTABLE foo
  #                               DEPENDENCIES foo)
endif()