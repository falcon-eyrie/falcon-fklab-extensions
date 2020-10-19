ADD_LIBRARY(eventconverter "eventconverter.cpp")
TARGET_LINK_LIBRARIES(eventconverter)

if(${TESTING})
ADD_EXECUTABLE(eventconverter_test "eventconvertertest.cpp")
endif()