FetchContent_Declare(
    catch2
    GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
    GIT_TAG         v2.13.4
    GIT_SHALLOW     true
)

set(CATCH_BUILD_TESTING OFF CACHE_INTERNAL "")
set(CATCH_BUILD_TESTING OFF CACHE_INTERNAL "")
set(CATCH_INSTALL_DOCS OFF CACHE_INTERNAL "")
set(BUILD_TESTING OFF CACHE_INTERNAL "")

FetchContent_MakeAvailable(catch2)

include_directories(
    "${catch2_SOURCE_DIR}/single_include")

# Append the contrib path to the module path list, so the include command can
# find the custom Catch module.
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/contrib")

include(CTest)
include(Catch)
