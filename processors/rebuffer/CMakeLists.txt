
ADD_LIBRARY(rebuffer "rebuffer.cpp")

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    TARGET_LINK_LIBRARIES(rebuffer PUBLIC utilities OpenMP::OpenMP_CXX)
else()
    TARGET_LINK_LIBRARIES(rebuffer utilities)
endif()
