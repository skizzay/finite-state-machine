file(GLOB _headers *.h)
add_library(skizzay_utils INTERFACE)
target_sources(skizzay_utils INTERFACE ${_headers})
