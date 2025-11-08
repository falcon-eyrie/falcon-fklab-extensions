
ADD_LIBRARY(multichannelfilter "multichannelfilter.cpp")
TARGET_LINK_LIBRARIES(multichannelfilter dsp)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    TARGET_LINK_LIBRARIES(multichannelfilter  PUBLIC dsp OpenMP::OpenMP_CXX)

else()
    TARGET_LINK_LIBRARIES(multichannelfilter dsp)
endif()
