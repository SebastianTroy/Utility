cmake_minimum_required(VERSION 3.21)

project(Utility LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

Message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

option(UTILITY_BuildTests "Build the unit tests if BUILD_TESTING is true." ON)
option(UTILITY_AutorunTests "Build & run the unit tests if BUILD_TESTING is true." OFF)

include(FetchContent)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include(json)
include(fmt)
include(catch2)

set(UTILITY_SOURCES
    ChromeTracing.cpp
    JsonHelpers.cpp
    NeuralNetwork.cpp
    NeuralNetworkConnector.cpp
    RangeConverter.cpp
    RollingStatistics.cpp
    Transform.cpp
    WindowedFrequencyStatistics.cpp
    WindowedRollingStatistics.cpp
)

set(UTILITY_HEADERS
    AutoClearingContainer.h
    Algorithm.h
    ChromeTracing.h
    CircularBuffer.h
    Concepts.h
    Energy.h
    FormatHelpers.h
    JsonHelpers.h
    JsonPolymorphicSerialisationHelper.h
    JsonSerialisationHelper.h
    MathConstants.h
    MinMax.h
    NeuralNetwork.h
    NeuralNetworkConnector.h
    QuadTree.h
    Random.h
    Range.h
    RangeConverter.h
    RollingStatistics.h
    Shape.h
    Transform.h
    TypeName.h
    WindowedFrequencyStatistics.h
    WindowedRollingStatistics.h
)

add_library(Utility OBJECT
    ${UTILITY_SOURCES}
    ${UTILITY_HEADERS}
)

target_link_libraries(Utility
    PRIVATE
    nlohmann_json::nlohmann_json
    fmt::fmt
)

if (BUILD_TESTING AND UTILITY_BuildTests)
    add_subdirectory(test)
endif()
