FetchContent_Declare(
    fmt
    GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
    GIT_TAG         7.1.3
    GIT_SHALLOW     true
)

set(FMT_DOC OFF CACHE INTERNAL "")
set(FMT_INSTALL OFF CACHE INTERNAL "")
set(FMT_TEST OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(fmt)

include_directories(
    "${fmt_SOURCE_DIR}/include"
)
