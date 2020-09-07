#cmake_minimum_required (VERSION 2.8)
#ENABLE_LANGUAGE(CXX)

project(nlxtestbench)

include_directories("${CMAKE_SOURCE_DIR}/lib")

add_executable(nlxtestbench main.cpp datasource.cpp datastreamer.cpp filesource.cpp whitenoisesource.cpp squaresource.cpp sinesource.cpp ripplesource.cpp)
target_link_libraries(nlxtestbench utilities options neuralynx yaml-cpp)


install(TARGETS nlxtestbench RUNTIME DESTINATION bin)
