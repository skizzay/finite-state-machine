add_executable(fsm_unit_tests)
find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(fsm_unit_tests fsm Catch2::Catch2)
set_property(TARGET fsm_unit_tests PROPERTY CXX_STANDARD 20)
target_include_directories(fsm_unit_tests PRIVATE ${PROJECT_SOURCE_DIR}/src/main/cpp)
target_sources(fsm_unit_tests PRIVATE main.cpp
  skizzay/fsm/action_transition.t.cpp
  skizzay/fsm/ancestors.t.cpp
  skizzay/fsm/flat_state_container.t.cpp
  skizzay/fsm/guarded_action_transition.t.cpp
  skizzay/fsm/guarded_transition.t.cpp
  skizzay/fsm/hierarchical_state_container.t.cpp
  skizzay/fsm/simple_transition.t.cpp
  skizzay/fsm/single_state_container.t.cpp
  skizzay/fsm/traits.t.cpp
  skizzay/fsm/type_list.t.cpp
)


  include(CTest)
  include(Catch)
  catch_discover_tests(fsm_unit_tests)
  enable_testing()
