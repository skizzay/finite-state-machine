add_library(fsm INTERFACE)
target_include_directories(fsm INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/main/cpp>
  $<INSTALL_INTERFACE:include>
)
target_sources(fsm INTERFACE
   skizzay/fsm/event.h
   skizzay/fsm/event_engine.h
   skizzay/fsm/event_handler.h
   skizzay/fsm/events_list.h
   skizzay/fsm/optional_reference.h
   skizzay/fsm/overload.h
   skizzay/fsm/state.h
   skizzay/fsm/state_accessible.h
   skizzay/fsm/state_provider.h
   skizzay/fsm/state_queryable.h
   skizzay/fsm/states_list.h
   skizzay/fsm/transition.h
   skizzay/fsm/type_list.h
)
