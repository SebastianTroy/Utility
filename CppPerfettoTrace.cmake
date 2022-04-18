# Add this file to your project directory, named "CppPerfettoTrace.cmake"
FetchContent_Declare(
    CppPerfettoTrace
    GIT_REPOSITORY  https://github.com/SebastianTroy/CppPerfettoTrace
    GIT_TAG         origin/main
)

set(CPP_PERFETTO_TRACE_BuildExample OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(CppPerfettoTrace)

include_directories(
    "${CppPerfettoTrace_SOURCE_DIR}"
)
