file(GLOB _headers *.h)
add_library(skizzay_fsm INTERFACE)
target_link_libraries(skizzay_fsm
   INTERFACE skizzay_utils
   )
target_sources(skizzay_fsm INTERFACE ${_headers})
