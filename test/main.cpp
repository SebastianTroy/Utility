#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <chrono>
#include <iostream>

int main(int argc, char* argv[])
{
    auto begin = std::chrono::steady_clock::now();

    int result = Catch::Session().run( argc, argv );

    auto end = std::chrono::steady_clock::now();
    std::cout << "[[ Tests ran for " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " milliseconds. ]]\n\n";

    return result;
}
