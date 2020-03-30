#include_directories( "../../ext" )
#include_directories( "../../lib" )
add_library(dio dio.cpp dummydio.cpp)
target_link_libraries( dio utilities )
