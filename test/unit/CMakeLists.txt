add_executable(fsm_unit_tests
   ${PROJECT_SOURCE_DIR}/src/test_main.cpp
   dispatcher.t.cpp
   state_container.t.cpp
   traits.t.cpp
   )
target_link_libraries(fsm_unit_tests CONAN_PKG::catch2 skizzay_fsm)
add_test(
   NAME fsm_unit_tests
   COMMAND fsm_unit_tests
   )
