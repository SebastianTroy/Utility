FetchContent_Declare(
    CppEasySerDes
    GIT_REPOSITORY  https://github.com/SebastianTroy/CppEasySerDes
    GIT_TAG         origin/NlohmannDirect
)

set(CPP_EASY_SERDES_BuildTests OFF CACHE INTERNAL "")
set(CPP_EASY_SERDES_AutorunTests OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(CppEasySerDes)

include_directories(
    "${CppEasySerDes_SOURCE_DIR}"
)
