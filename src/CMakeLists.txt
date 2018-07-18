file(GLOB _headers src/*.h)
add_library(skizzay_fsm INTERFACE)
target_sources(skizzay_fsm INTERFACE ${_headers})
