ADD_LIBRARY(eventdelayed "eventdelayed.cpp")
TARGET_LINK_LIBRARIES(eventdelayed utilities)

add_executable(eventdelayed_test eventdelayed_test.cpp)