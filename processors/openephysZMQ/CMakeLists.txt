add_library(openephysZMQ "openephysZMQ.cpp")

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/channel_generated.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/channel.fbs
    COMMAND flatc --cpp ${CMAKE_CURRENT_SOURCE_DIR}/channel.fbs
)

add_custom_target(channelbuffer DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/channel_generated.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(openephysZMQ  flatbuffers)
add_dependencies(openephysZMQ channelbuffer)
