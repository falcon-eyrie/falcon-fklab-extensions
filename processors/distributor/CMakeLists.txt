find_package(OpenMP REQUIRED)
ADD_LIBRARY(distributor "distributor.cpp")
TARGET_LINK_LIBRARIES(distributor PUBLIC OpenMP::OpenMP_CXX utilities neuralynx)
