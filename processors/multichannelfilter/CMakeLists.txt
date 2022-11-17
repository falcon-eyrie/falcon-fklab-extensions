
ADD_LIBRARY(multichannelfilter "multichannelfilter.cpp")
TARGET_LINK_LIBRARIES(multichannelfilter dsp)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(multichannelfilter  PUBLIC dsp OpenMP::OpenMP_CXX)

else()
    TARGET_LINK_LIBRARIES(multichannelfilter dsp)
endif()
