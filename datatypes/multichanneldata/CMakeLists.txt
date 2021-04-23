ADD_LIBRARY(multichanneldata INTERFACE)
target_sources(multichanneldata INTERFACE "multichanneldata.hpp")
target_include_directories( multichanneldata  INTERFACE ../../lib/flatbuffers)
