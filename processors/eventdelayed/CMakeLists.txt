ADD_LIBRARY(eventdelayed "eventdelayed.cpp")
TARGET_LINK_LIBRARIES(eventdelayed utilities)


if (${TESTING})
    add_executable(eventdelayed_test eventdelayed_test.cpp)
endif()