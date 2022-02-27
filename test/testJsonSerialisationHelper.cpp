#include "testJsonSerialisationHelper.h"

#include <catch2/catch.hpp>

using namespace nlohmann;

TEST_CASE("TrivialTestType", "[json]")
{
    TrivialTestType original { 79, { 42, 44, 79 }, "foobar" };

    json serialised = util::JsonSerialisationHelper<TrivialTestType>::Serialise(original);
    REQUIRE(util::JsonSerialisationHelper<TrivialTestType>::Validate(serialised));

    TrivialTestType deserialised = util::JsonSerialisationHelper<TrivialTestType>::Deserialise(serialised);
    json deserialisedReserialised = util::JsonSerialisationHelper<TrivialTestType>::Serialise(deserialised);

    // test this first so failures print out the before and after JSON, instead of a less helpful failed comparison
    REQUIRE(deserialisedReserialised == serialised);
    REQUIRE(deserialised == original);
}


TEST_CASE("TestNonTrivial", "[json]")
{
    NonTrivialTestType original { 77 };
    original.b_ = false;

    json serialised = util::JsonSerialisationHelper<NonTrivialTestType>::Serialise(original);
    REQUIRE(util::JsonSerialisationHelper<NonTrivialTestType>::Validate(serialised));

    NonTrivialTestType deserialised = util::JsonSerialisationHelper<NonTrivialTestType>::Deserialise(serialised);
    json deserialisedReserialised = util::JsonSerialisationHelper<NonTrivialTestType>::Serialise(deserialised);

    // test this first so failures print out the before and after JSON, instead of a less helpful failed comparison
    REQUIRE(deserialisedReserialised == serialised);
    REQUIRE(deserialised == original);
}
