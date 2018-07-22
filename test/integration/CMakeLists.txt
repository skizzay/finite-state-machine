add_executable(fsm_integration_tests
   ${PROJECT_SOURCE_DIR}/src/test_main.cpp
   actionable_transitions.t.cpp
   guarded_transitions.t.cpp
   simple_transitions.t.cpp
   traffic_light.t.cpp
   )
target_link_libraries(fsm_integration_tests CONAN_PKG::catch2 skizzay_fsm)
add_test(
   NAME fsm_integration_tests
   COMMAND fsm_integration_tests
   )
