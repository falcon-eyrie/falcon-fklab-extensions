add_library(dsp "filter.cpp" "algorithms.cpp" "gram_savitzky_golay.cpp")
target_link_libraries(dsp yaml-cpp)
