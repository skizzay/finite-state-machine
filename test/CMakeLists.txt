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
target_link_libraries(skizzay_fsm_tests CONAN_PKG::catch2 skizzay_fsm)
catch_discover_tests(skizzay_fsm_tests)
