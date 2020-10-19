
ADD_LIBRARY(eventfilter "eventfilter.cpp")
TARGET_LINK_LIBRARIES(eventfilter utilities)

if(${TESTING})
add_executable(eventfilter_test eventfiltertest.cpp)
endif()