
ADD_LIBRARY(burstdetector "burstdetector.cpp")
TARGET_LINK_LIBRARIES(burstdetector dsp)


ADD_EXECUTABLE(burstdetector_test "burstdetectortest.cpp")
