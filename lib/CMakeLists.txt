
add_subdirectory( dsp )
add_subdirectory( neuralynx )
add_subdirectory( dio )
add_subdirectory( serialib )
add_subdirectory( recording_utilities )
add_subdirectory( math_expression_parser )

if (${TESTING})
    add_subdirectory( tests )
endif()
