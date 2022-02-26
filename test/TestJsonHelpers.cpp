#include <Algorithm.h>
#include <FormatHelpers.h>
#include <JsonHelpers.h>
#include <JsonPolymorphicSerialisationHelper.h>
#include <JsonSerialisationHelper.h>
#include <TypeName.h>
#include <Concepts.h>
#include <Random.h>

#include <catch2/catch.hpp>

#include <string>
#include <vector>
#include <utility>
#include <map>
#include <iostream>

using namespace nlohmann;

/*
 * Some Types for testing
 */

enum TestEnum {
    First,
    Middle,
    Last,
};

enum class TestEnumClass : short {
    First,
    Middle,
    Last,
};


template <typename A, typename B, typename C>
class TrivialTestType {
    static_assert(!IsAllSameType<A, B, C>, "All types must be unique!");
public:
    A a_;
    B b_;
    C c_;

    static void ConfigureJsonSerialisationHelper(Tril::JsonSerialisationHelper<TrivialTestType<A, B, C>>& helper)
    {
        helper.RegisterVariable(std::string(Tril::TypeName<A>()), &TrivialTestType::a_);
        helper.RegisterVariable(std::string(Tril::TypeName<B>()), &TrivialTestType::b_);
        helper.RegisterVariable(std::string(Tril::TypeName<C>()), &TrivialTestType::c_);
    }

    bool operator==(const TrivialTestType& other) const
    {
        bool aMatches = false;
        if constexpr (IsSharedPointer<A>) {
            aMatches = *a_ == *other.a_;
        } else {
            aMatches = a_ == other.a_;
        }

        bool bMatches = false;
        if constexpr (IsSharedPointer<B>) {
            bMatches = *b_ == *other.b_;
        } else {
            bMatches = b_ == other.b_;
        }

        bool cMatches = false;
        if constexpr (IsSharedPointer<C>) {
            cMatches = *c_ == *other.c_;
        } else {
            cMatches = c_ == other.c_;
        }

        return aMatches && bMatches && cMatches;
    }

    bool operator!=(const TrivialTestType& other) const
    {
        return !this->operator==(other);
    }
};

template <typename A, typename B, typename C>
class NonTrivialTestType {
    static_assert(!IsAllSameType<A, B, C>, "All types must be unique!");
public:
    C c_;

    NonTrivialTestType(const NonTrivialTestType<A, B, C>& other) = default;
    NonTrivialTestType(NonTrivialTestType<A, B, C>&& other) = default;

    NonTrivialTestType(A a, B b)
        : a_(a)
        , b_(b)
        , c_{}
    {
    }

    static void ConfigureJsonSerialisationHelper(Tril::JsonSerialisationHelper<NonTrivialTestType<A, B, C>>& helper)
    {
        helper.RegisterConstructor(
                    helper.template CreateParameter<A>(std::string(Tril::TypeName<A>()), [](const NonTrivialTestType& instance) -> A { return instance.GetA(); }),
                    helper.template CreateParameter<B>(std::string(Tril::TypeName<B>()), &NonTrivialTestType::b_) // Still works even though member is private!
                    );
        helper.RegisterVariable(std::string(Tril::TypeName<C>()), &NonTrivialTestType::c_);
    }

    const A& GetA() const
    {
        return a_;
    }

    const B& GetB() const
    {
        return b_;
    }

    bool operator==(const NonTrivialTestType& other) const
    {
        bool aMatches = false;
        if constexpr (IsSharedPointer<A>) {
            aMatches = *a_ == *other.a_;
        } else {
            aMatches = a_ == other.a_;
        }

        bool bMatches = false;
        if constexpr (IsSharedPointer<B>) {
            bMatches = *b_ == *other.b_;
        } else {
            bMatches = b_ == other.b_;
        }

        bool cMatches = false;
        if constexpr (IsSharedPointer<C>) {
            cMatches = *c_ == *other.c_;
        } else {
            cMatches = c_ == other.c_;
        }

        return aMatches && bMatches && cMatches;
    }

    bool operator!=(const NonTrivialTestType& other) const
    {
        return !this->operator==(other);
    }

private:
    const A a_;
    const B b_;
};

class BaseTestType {
public:

    BaseTestType(bool fact)
        : fact_(fact)
    {
    }

    virtual std::string TypeName() const = 0;
    virtual bool operator==(const BaseTestType& other) const = 0;
    virtual bool operator!=(const BaseTestType& other) const = 0;

    bool GetFact() const
    {
        return fact_;
    }

private:
    bool fact_;
};

class ChildTestTypeA : public BaseTestType {
public:
    ChildTestTypeA(int num, bool fact)
        : BaseTestType(fact)
        , num_(num)
    {
    }

    virtual std::string TypeName() const override
    {
        return std::string(Tril::TypeName<ChildTestTypeA>());
    }

    static void ConfigureJsonSerialisationHelper(Tril::JsonSerialisationHelper<ChildTestTypeA>& helper)
    {
        helper.RegisterConstructor(
                    helper.CreateParameter<int>("num", &ChildTestTypeA::num_), // Still works even though member is private!
                    helper.CreateParameter<bool>("fact", [](const ChildTestTypeA& instance) -> bool { return instance.GetFact(); })
                    );
    }

    bool operator==(const BaseTestType& other) const override
    {
        const ChildTestTypeA* otherPtr = dynamic_cast<const ChildTestTypeA*>(&other);
        return otherPtr && num_ == otherPtr->num_ && GetFact() == otherPtr->GetFact();
    }

    bool operator!=(const BaseTestType& other) const override
    {
        return !this->operator==(other);
    }

    int GetNumber() const
    {
        return num_;
    }

private:
    int num_;
};

class ChildTestTypeB : public BaseTestType {
public:
    ChildTestTypeB(double num, bool fact)
        : BaseTestType(fact)
        , num_(num)
    {
    }

    virtual std::string TypeName() const override
    {
        return std::string(Tril::TypeName<ChildTestTypeB>());
    }

    static void ConfigureJsonSerialisationHelper(Tril::JsonSerialisationHelper<ChildTestTypeB>& helper)
    {
        helper.RegisterConstructor(
                    helper.CreateParameter<double>("num", &ChildTestTypeB::num_), // Still works even though member is private!
                    helper.CreateParameter<bool>("fact", [](const ChildTestTypeB& instance) -> bool { return instance.GetFact(); })
                    );
    }

    bool operator==(const BaseTestType& other) const override
    {
        const ChildTestTypeB* otherPtr = dynamic_cast<const ChildTestTypeB*>(&other);
        return otherPtr && num_ == otherPtr->num_ && GetFact() == otherPtr->GetFact();
    }

    bool operator!=(const BaseTestType& other) const override
    {
        return !this->operator==(other);
    }

    double GetNumber() const
    {
        return num_;
    }

private:
    double num_;
};

/*
 * Some operators to make testing easier
 */

std::ostream& operator<<(std::ostream& ostr, const TestEnum& value)
{
    switch (value) {
        case TestEnum::First : return ostr << "TestEnum::First";
        case TestEnum::Middle : return ostr << "TestEnum::Middle";
        case TestEnum::Last : return ostr << "TestEnum::Last";
    }
    return ostr << "TestEnum::INVALID";
}

std::ostream& operator<<(std::ostream& ostr, const TestEnumClass& value)
{
    switch (value) {
    case TestEnumClass::First : return ostr << "TestEnum::First";
    case TestEnumClass::Middle : return ostr << "TestEnum::Middle";
    case TestEnumClass::Last : return ostr << "TestEnum::Last";
    }
    return ostr << "TestEnum::INVALID";
}

template <typename A, typename B, typename C>
std::ostream& operator<<(std::ostream& ostr, const TrivialTestType<A, B, C>& value)
{
    return ostr << Tril::TypeName<TrivialTestType<A, B, C>>() << "{" << value.a_ << ", " << value.b_ << ", " << value.c_ << "}";
}

template <typename A, typename B, typename C>
std::ostream& operator<<(std::ostream& ostr, const NonTrivialTestType<A, B, C>& value)
{
    return ostr << Tril::TypeName<NonTrivialTestType<A, B, C>>() << "{" << value.GetA() << ", " << value.GetB() << ", " << value.c_ << "}";
}

std::ostream& operator<<(std::ostream& ostr, const ChildTestTypeA& value)
{
    return ostr << Tril::TypeName<ChildTestTypeA>() << "{" << value.GetNumber() << ", " << value.GetFact() << "}";
}

std::ostream& operator<<(std::ostream& ostr, const ChildTestTypeB& value)
{
    return ostr << Tril::TypeName<ChildTestTypeB>() << "{" << value.GetNumber() << ", " << value.GetFact() << "}";
}

namespace {

template <typename A, typename B, typename C>
void TestTrivial(A a, B b, C c)
{
    TrivialTestType<A, B, C> original { a, b, c };

    json serialised = Tril::JsonSerialisationHelper<TrivialTestType<A, B, C>>::Serialise(original);
    REQUIRE(Tril::JsonSerialisationHelper<TrivialTestType<A, B, C>>::Validate(serialised));

    TrivialTestType<A, B, C> deserialised = Tril::JsonSerialisationHelper<TrivialTestType<A, B, C>>::Deserialise(serialised);
    json deserialisedReserialised = Tril::JsonSerialisationHelper<TrivialTestType<A, B, C>>::Serialise(deserialised);

    // test this first so failures print out the before and after JSON, instead of a less helpful failed comparison
    REQUIRE(deserialisedReserialised == serialised);
    REQUIRE(deserialised == original);
}

template <typename A, typename B, typename C>
void TestNonTrivial(A a, B b, C c)
{
    NonTrivialTestType<A, B, C> original { a, b };
    original.c_ = c;

    json serialised = Tril::JsonSerialisationHelper<NonTrivialTestType<A, B, C>>::Serialise(original);
    REQUIRE(Tril::JsonSerialisationHelper<NonTrivialTestType<A, B, C>>::Validate(serialised));

    NonTrivialTestType<A, B, C> deserialised = Tril::JsonSerialisationHelper<NonTrivialTestType<A, B, C>>::Deserialise(serialised);
    json deserialisedReserialised = Tril::JsonSerialisationHelper<NonTrivialTestType<A, B, C>>::Serialise(deserialised);

    // test this first so failures print out the before and after JSON, instead of a less helpful failed comparison
    REQUIRE(deserialisedReserialised == serialised);
    REQUIRE(deserialised == original);
}

template <typename A, typename B, typename C>
void RunTest(A a, B b, C c)
{
    TestTrivial(a, b, c);
    TestNonTrivial(a, b, c);
}

} // end anonymous namespace

TEST_CASE("Basic Type", "[json]")
{
    RunTest(42, true, std::string("Foo"));
}

TEST_CASE("pair", "[json]")
{
    SECTION("pair<int, int>")
    {
        RunTest(std::make_pair(42, 79), true, std::string("Foo"));
    }
    SECTION("pair<int, string>")
    {
        RunTest(std::make_pair(42, std::string("Hello World!")), true, std::string("Foo"));
    }
    SECTION("pair<int, pair<bool, double>>")
    {
        RunTest(std::make_pair(444, std::make_pair(true, 3.14)), true, std::string("Foo"));
    }
}

TEST_CASE("Nested Type", "[json]")
{
    RunTest(TrivialTestType<bool, float, double>{ false, 0.23f, 1234.5678 }, true, std::string("Foo"));

    // Cannot run this test, as it requires TestNonTrivialType to be copy assignable
    // TestNonTrivialType<bool, float, double> nonTrivial = TestNonTrivialType<bool, float, double>(false, 0.23f);
    // nonTrivial.c_ = 1234.5678;
    // RunTest(std::move(nonTrivial), true, std::string("Foo"));
}

TEST_CASE("Type with Container", "[json]")
{
    SECTION("vector<int>")
    {
        RunTest(std::vector<int>{ 0, 1, 2, 3, 4, 27 }, true, 0.123f);
    }
    SECTION("vector<string>")
    {
        RunTest(std::vector<std::string>{ "aaa", "bcd", "Hello World!" }, false, 1234.9876f);
    }
    SECTION("map<int, bool>")
    {
        RunTest(std::map<int, bool>{ {42, true}, {0, false}, {79, true} }, true, 0.123f);
    }
    SECTION("map<int, string>")
    {
        RunTest(std::map<int, std::string>{ {42, "meaning"}, {0, "null"}, {79, "nsfw?"} }, true, 0.123f);
    }
    SECTION("vector<TestPod<int, bool, double>>")
    {
        RunTest(TrivialTestType<int, bool, double>{ 44, true, 981234.5321 }, false, 1234.9876f);

        std::vector<NonTrivialTestType<int, bool, double>> nonTrivials;
        for (int i = 0; i < 5; ++i) {
            nonTrivials.emplace_back(Random::Number<int>(-100, 100), Random::Boolean());
            nonTrivials.back().c_ = Random::Number<double>(-1e7, 1e7);
        }
        RunTest(std::move(nonTrivials), false, 1234.9876f);
    }
}

TEST_CASE("Nested Maps", "[json]")
{
    Random::Seed(79);
    auto createMap = []() -> std::map<double, bool>
    {
        std::map<double, bool> map;
        for (int i = 0; i < 5; ++i) {
            map.insert({Random::Number<double>(-10.0, 10.0), Random::Boolean()});
        }
        return map;
    };

    SECTION("map<int, map<double, bool>>")
    {
        RunTest(std::map<int, std::map<double, bool>>{ {0, createMap()}, {42, createMap()}, {44, createMap()}, {79, createMap()} }, true, 0.123f);
    }

    SECTION("vector<map<double, bool>>")
    {
        RunTest(std::vector<std::map<double, bool>>{ createMap(), createMap(), createMap(), createMap(), createMap() }, true, 0.123f);
    }
}

TEST_CASE("Nested Vectors", "[json]")
{
    SECTION("vector<vector<int>>")
    {
        RunTest(std::vector<std::vector<int>>{{ 0, 1, 2, 3, 4, 27 }, { -44, 79, 180 }}, true, 0.123f);
    }
    SECTION("vector<vector<string>>")
    {
        RunTest(std::vector<std::vector<std::string>>{ { "aaa", "bcd", "Hello World!" }, { "seconda", "secondb" }, { "third", "hehe Richard...", "foo" } }, false, 1234.9876f);
    }
    SECTION("vector<vector<TestPod<int, bool, double>>>")
    {
        RunTest(std::vector<TrivialTestType<int, bool, double>>{ { 44, true, 981234.5321 }, { 79, false, 144.4 }, { -384756, true, -1e7 } }, false, 1234.9876f);

        std::vector<std::vector<NonTrivialTestType<int, bool, double>>> vectorOfVectorsOfNonTrivials;
        for (int i = 0; i < 5; ++i) {
            std::vector<NonTrivialTestType<int, bool, double>> vectorOfNonTrivials;
            for (int i = 0; i < 5; ++i) {
                vectorOfNonTrivials.emplace_back(Random::Number<int>(-100, 100), Random::Boolean());
                vectorOfNonTrivials.back().c_ = Random::Number<double>(-1e7, 1e7);
            }
        vectorOfVectorsOfNonTrivials.push_back(std::move(vectorOfNonTrivials));
        }
        RunTest(std::move(vectorOfVectorsOfNonTrivials), false, 1234.9876f);
    }
}

TEST_CASE("Enumerations", "[json]")
{
    SECTION("enum")
    {
        RunTest(42, true, TestEnum::Middle);
    }

    SECTION("enum class : short")
    {
        RunTest(42, true, TestEnumClass::Last);
    }

    SECTION("enum, in vector")
    {
        RunTest(42, true, std::vector<TestEnum>{ TestEnum::Last, TestEnum::First, TestEnum::First, TestEnum::Middle, TestEnum::Middle });
    }

    SECTION("enum class : short, in vector")
    {
        RunTest(42, true, std::vector<TestEnumClass>{ TestEnumClass::Last, TestEnumClass::First, TestEnumClass::First, TestEnumClass::Middle, TestEnumClass::Middle });
    }
}

TEST_CASE("std::shared_ptr", "[json]")
{
    SECTION("std::shared_ptr<int>")
    {
        RunTest(std::make_shared<int>(79), false, std::string("Bah"));
    }

    SECTION("std::shared_ptr<std::string>")
    {
        RunTest(std::make_shared<std::string>("Foo"), 79, false);
    }

    SECTION("std::shared_ptr<enum>")
    {
        RunTest(42, true, std::make_shared<TestEnum>(TestEnum::Middle));
    }

    SECTION("std::shared_ptr<enum class : short>")
    {
        RunTest(42, true, std::make_shared<TestEnumClass>(TestEnumClass::Last));
    }

    SECTION("std::shared_ptr<std::vector<int>>")
    {
        RunTest(42, true, std::make_shared<std::vector<int>>(std::initializer_list<int>{ 42, 44, 79 }));
    }

    SECTION("std::shared_ptr<JsonSerialisableType>")
    {
        std::shared_ptr<TrivialTestType<int, bool, double>> trivialPtr = std::make_shared<TrivialTestType<int, bool, double>>();
        trivialPtr->a_ = -384756;
        trivialPtr->b_ = true;
        trivialPtr->c_ = -1e7;
        std::shared_ptr<NonTrivialTestType<int, bool, double>> nonTrivialPtr = std::make_shared<NonTrivialTestType<int, bool, double>>(-384756, true);
        nonTrivialPtr->c_ = -1e7;
        RunTest(trivialPtr, false, 1234.9876f);
        RunTest(nonTrivialPtr, false, 1234.9876f);
    }
}

TEST_CASE("Polymorphism", "[json]")
{
    Tril::JsonPolymorphicSerialisationHelper<BaseTestType>::template RegisterChildType<ChildTestTypeA>();
    Tril::JsonPolymorphicSerialisationHelper<BaseTestType>::template RegisterChildType<ChildTestTypeB>();

    ChildTestTypeA originalA { 42, true };
    ChildTestTypeB originalB { 79.44, true };

    json serialisedA = Tril::JsonPolymorphicSerialisationHelper<BaseTestType>::Serialise(originalA);
    json serialisedB = Tril::JsonPolymorphicSerialisationHelper<BaseTestType>::Serialise(originalB);
    REQUIRE(Tril::JsonPolymorphicSerialisationHelper<BaseTestType>::Validate(serialisedA));
    REQUIRE(Tril::JsonPolymorphicSerialisationHelper<BaseTestType>::Validate(serialisedB));

    ChildTestTypeA traditionallyDeserialisedA = Tril::JsonSerialisationHelper<ChildTestTypeA>::Deserialise(serialisedA);
    ChildTestTypeB traditionallyDeserialisedB = Tril::JsonSerialisationHelper<ChildTestTypeB>::Deserialise(serialisedB);
    std::shared_ptr<BaseTestType> polymorphicallyDeserialisedA = Tril::JsonPolymorphicSerialisationHelper<BaseTestType>::Deserialise(serialisedA);
    std::shared_ptr<BaseTestType> polymorphicallyDeserialisedB = Tril::JsonPolymorphicSerialisationHelper<BaseTestType>::Deserialise(serialisedB);
    json deserialisedReserialisedA = Tril::JsonPolymorphicSerialisationHelper<BaseTestType>::Serialise(*polymorphicallyDeserialisedA);
    json deserialisedReserialisedB = Tril::JsonPolymorphicSerialisationHelper<BaseTestType>::Serialise(*polymorphicallyDeserialisedB);

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

    RunTest(basePtrA, true, 444);
    RunTest(basePtrB, true, 444);
}
