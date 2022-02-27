if (MINGW)
    # This allows Catch2 to be built without MinGW having a fit over the size of the object file generated
    add_compile_options("-Wa,-mbig-obj")
elseif(MSVC)
    add_compile_options("-bigobj")
    set(_FS_C " /FS")
endif()

add_executable(Tests)

target_sources(Tests
    PUBLIC
    main.cpp
    TestCircularBuffer.cpp
    TestJsonHelpers.cpp
    testJsonSerialisationHelper.h
    testJsonSerialisationHelper.cpp
    testJsonPolymorphicSerialisationHelper.h
    testJsonPolymorphicSerialisationHelper.cpp
    TestShape.cpp
    TestQuadTree.cpp
    TestRangeConverter.cpp
    TestRollingStatistics.cpp
    TestTransform.cpp
    TestWindowedFrequencyStatistics.cpp
    TestWindowedRollingStatistics.cpp
)

target_include_directories(Tests
    PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Utility
)

target_link_libraries(Tests
    PRIVATE
    Catch2::Catch2
    Utility
)

add_custom_command(TARGET Tests POST_BUILD
    COMMAND ${CMAKE_BINARY_DIR}/test/Tests
    COMMENT "Running Tests..."
)
