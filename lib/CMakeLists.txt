
add_subdirectory( dsp )
add_subdirectory( neuralynx )
add_subdirectory( dio )
add_subdirectory( serialib )
add_subdirectory( recording_utilities )

if (${TESTING})
    add_subdirectory( tests )
endif()
