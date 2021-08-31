add_library(fsm INTERFACE)
target_include_directories(fsm INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/main/cpp>
  $<INSTALL_INTERFACE:include>
)
target_sources(fsm INTERFACE
   skizzay/fsm/action_transition.h
   skizzay/fsm/ancestors.h
   skizzay/fsm/basic_state_container.h
   skizzay/fsm/concepts.h
   skizzay/fsm/enter.h
   skizzay/fsm/event.h
   skizzay/fsm/exit.h
   skizzay/fsm/finite_state_machine.h
   skizzay/fsm/flat_state_container.h
   skizzay/fsm/guarded_action_transition.h
   skizzay/fsm/guarded_transition.h
   skizzay/fsm/hierarchical_state_container.h
   skizzay/fsm/machine_as_state_container.h
   skizzay/fsm/optional_reference.h
   skizzay/fsm/orthogonal_state_container.h
   skizzay/fsm/overload.h
   skizzay/fsm/simple_transition.h
   skizzay/fsm/single_state_container.h
   skizzay/fsm/traits.h
   skizzay/fsm/transition_coordinator.h
   skizzay/fsm/trigger.h
   skizzay/fsm/type_list.h
)
