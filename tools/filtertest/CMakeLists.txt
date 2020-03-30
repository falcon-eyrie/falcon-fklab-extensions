project( filtertest )

#set(CMAKE_CXX_FLAGS "-O3 -Wall -Wunused -std=c++11")

include_directories( "${CMAKE_SOURCE_DIR}/lib" )

add_executable( filtertest filtertest.cpp )
target_link_libraries (filtertest utilities dsp)

install(TARGETS filtertest RUNTIME DESTINATION bin)
