
MESSAGE("Build TEST")
project(test)
include(CTest)
enable_testing()

FetchContent_Declare(
	googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG        release-1.10.0
)
FetchContent_MakeAvailable(googletest)

MESSAGE("Add tests for fklab extension: recording utilities lib test")
add_executable(recording_lib_test tests_recording_utilities.cpp )
target_link_libraries(recording_lib_test gtest gmock yaml-cpp utilities)
add_test(NAME recording_lib_test COMMAND recording_lib_test)

