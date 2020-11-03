
ADD_LIBRARY(event2state "event2state.cpp")
TARGET_LINK_LIBRARIES(event2state)

if (${TESTING})
    add_executable(event2state_test event2state_test.cpp)
endif()