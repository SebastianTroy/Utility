#include "testJsonPolymorphicSerialisationHelper.h"

#include <catch2/catch.hpp>

using namespace nlohmann;

TEST_CASE("Polymorphism", "[json]")
{
    util::JsonPolymorphicSerialisationHelper<BaseTestType>::template RegisterChildType<ChildTestTypeA>();
    util::JsonPolymorphicSerialisationHelper<BaseTestType>::template RegisterChildType<ChildTestTypeB>();

    ChildTestTypeA originalA { 42, true };
    ChildTestTypeB originalB { 79.44, true };

    json serialisedA = util::JsonPolymorphicSerialisationHelper<BaseTestType>::Serialise(originalA);
    json serialisedB = util::JsonPolymorphicSerialisationHelper<BaseTestType>::Serialise(originalB);
    REQUIRE(util::JsonPolymorphicSerialisationHelper<BaseTestType>::Validate(serialisedA));
    REQUIRE(util::JsonPolymorphicSerialisationHelper<BaseTestType>::Validate(serialisedB));

    ChildTestTypeA traditionallyDeserialisedA = util::JsonSerialisationHelper<ChildTestTypeA>::Deserialise(serialisedA);
    ChildTestTypeB traditionallyDeserialisedB = util::JsonSerialisationHelper<ChildTestTypeB>::Deserialise(serialisedB);
    std::shared_ptr<BaseTestType> polymorphicallyDeserialisedA = util::JsonPolymorphicSerialisationHelper<BaseTestType>::Deserialise(serialisedA);
    std::shared_ptr<BaseTestType> polymorphicallyDeserialisedB = util::JsonPolymorphicSerialisationHelper<BaseTestType>::Deserialise(serialisedB);
    json deserialisedReserialisedA = util::JsonPolymorphicSerialisationHelper<BaseTestType>::Serialise(*polymorphicallyDeserialisedA);
    json deserialisedReserialisedB = util::JsonPolymorphicSerialisationHelper<BaseTestType>::Serialise(*polymorphicallyDeserialisedB);

    // test this first so failures print out the before and after JSON, instead of a less helpful failed comparison
    REQUIRE(originalA == traditionallyDeserialisedA);
    REQUIRE(originalB == traditionallyDeserialisedB);
    REQUIRE(serialisedA != serialisedB);
    REQUIRE(deserialisedReserialisedA == serialisedA);
    REQUIRE(deserialisedReserialisedB == serialisedB);
    REQUIRE(*std::dynamic_pointer_cast<ChildTestTypeA>(polymorphicallyDeserialisedA) == originalA);
    REQUIRE(*std::dynamic_pointer_cast<ChildTestTypeB>(polymorphicallyDeserialisedB) == originalB);
}

TEST_CASE("Nested", "[json]")
{
    std::shared_ptr<BaseTestType> basePtrA = std::make_shared<ChildTestTypeA>(42, false);
    std::shared_ptr<BaseTestType> basePtrB = std::make_shared<ChildTestTypeB>(-432.1235, true);

    NestedType originalA{ basePtrA };
    NestedType originalB{ basePtrB };

    json serialisedA = util::JsonSerialisationHelper<NestedType>::Serialise(originalA);
    json serialisedB = util::JsonSerialisationHelper<NestedType>::Serialise(originalB);
    REQUIRE(util::JsonSerialisationHelper<NestedType>::Validate(serialisedA));
    REQUIRE(util::JsonSerialisationHelper<NestedType>::Validate(serialisedB));

    NestedType deserialisedA = util::JsonSerialisationHelper<NestedType>::Deserialise(serialisedA);
    NestedType deserialisedB = util::JsonSerialisationHelper<NestedType>::Deserialise(serialisedB);

    REQUIRE(*std::dynamic_pointer_cast<ChildTestTypeA>(deserialisedA.polymorphic_) == *originalA.polymorphic_);
    REQUIRE(*std::dynamic_pointer_cast<ChildTestTypeB>(deserialisedB.polymorphic_) == *originalB.polymorphic_);
}
