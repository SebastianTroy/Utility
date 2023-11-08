#include <NeuralNetwork.h>
#include <NeuralNetworkConnector.h>

#include <Random.h>
#include <Algorithm.h>

using namespace util;

template<typename T>
bool operator==(const std::vector<T>& a, const std::vector<T>& b)
{
    bool allMatch = true;
    IterateBoth(a, b, [&](const T& aItem, const T& bItem)
    {
        allMatch = allMatch && (aItem == bItem);
    });
    return allMatch;
}

bool operator==(const NeuralNetwork& a, const NeuralNetwork& b)
{
    return a.GetLayers() == b.GetLayers();
}

bool operator==(const NeuralNetworkConnector& a, const NeuralNetworkConnector& b)
{
    return a.Inspect() == b.Inspect();
}

#include <catch2/catch.hpp>

template<typename TestType>
void Test(const TestType& toTest)
{
    auto serialised = esd::Serialise<TestType>(toTest);

    REQUIRE(esd::Validate<TestType>(serialised));

    auto deserialised = esd::DeserialiseWithoutChecks<TestType>(serialised);
    auto reserialised = esd::Serialise<TestType>(deserialised);

    REQUIRE(serialised == reserialised);
    REQUIRE(deserialised == toTest);
};

TEST_CASE("Serialiser<NeuralNetwork>", "[serialisation]")
{
    Random::Seed(8364592);

    NeuralNetwork randNetwork(4, 7, NeuralNetwork::InitialWeights::Random);
    NeuralNetwork passNetwork(4, 7, NeuralNetwork::InitialWeights::PassThrough);

    Test(randNetwork);
    Test(passNetwork);
}

TEST_CASE("Serialiser<NeuralNetworkConnector>", "[serialisation]")
{
    Random::Seed(8364592);

    NeuralNetworkConnector randConnector(4, 7);
    NeuralNetworkConnector passConnector(4, 7);

    Test(randConnector);
    Test(passConnector);
}
