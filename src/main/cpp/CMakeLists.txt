add_library(fsm INTERFACE)
target_include_directories(fsm INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/main/cpp>
  $<INSTALL_INTERFACE:include>
)
target_sources(fsm INTERFACE
   skizzay/fsm/event.h
   skizzay/fsm/optional_reference.h
   skizzay/fsm/overload.h
   skizzay/fsm/state.h
   skizzay/fsm/state_queryable.h
   skizzay/fsm/transition.h
   skizzay/fsm/type_list.h
)
