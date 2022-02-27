#include <JsonHelpers.h>
#include <Concepts.h>

#include <catch2/catch.hpp>

#include <string>
#include <vector>
#include <utility>
#include <map>
#include <set>
#include <iostream>
#include <limits>

using namespace nlohmann;

/*
 * Some Types for testing
 */

enum TestJsonHelpersEnum {
    First,
    Middle,
    Last,
};

enum class TestJsonHelpersEnumClass : short {
    First,
    Middle,
    Last,
};

/*
 * Some operators to make testing easier
 */

std::ostream& operator<<(std::ostream& ostr, const TestJsonHelpersEnum& value)
{
    switch (value) {
        case TestJsonHelpersEnum::First : return ostr << "TestJsonHelpersEnum::First";
        case TestJsonHelpersEnum::Middle : return ostr << "TestJsonHelpersEnum::Middle";
        case TestJsonHelpersEnum::Last : return ostr << "TestJsonHelpersEnum::Last";
    }
    return ostr << "TestEnum::INVALID";
}

std::ostream& operator<<(std::ostream& ostr, const TestJsonHelpersEnumClass& value)
{
    switch (value) {
    case TestJsonHelpersEnumClass::First : return ostr << "TestJsonHelpersEnumClass::First";
    case TestJsonHelpersEnumClass::Middle : return ostr << "TestJsonHelpersEnumClass::Middle";
    case TestJsonHelpersEnumClass::Last : return ostr << "TestJsonHelpersEnumClass::Last";
    }
    return ostr << "TestEnum::INVALID";
}

namespace {
    template <typename T>
    void RunTest(T value)
    {
        auto serialised = JsonHelpers::Serialise(value);
        T deserialised = JsonHelpers::Deserialise<T>(serialised);
        auto reSerialised = JsonHelpers::Serialise(deserialised);

        // Check this first so that an error displays nicely comparable json structures
        REQUIRE(serialised == reSerialised);
        if constexpr (IsSharedPointer<T>) {
            REQUIRE(*deserialised == *value);
        } else {
            REQUIRE(deserialised == value);
        }
        REQUIRE(JsonHelpers::Validate<T>(serialised));
        REQUIRE(JsonHelpers::Validate<T>(reSerialised));
    }
} // end anonymous namespace

TEST_CASE("Simple Types", "[json]")
{
    RunTest(std::numeric_limits<int>::lowest());
    RunTest(std::numeric_limits<int>::min());
    RunTest(42);
    RunTest(-420);
    RunTest(std::numeric_limits<int>::max());

    RunTest(std::numeric_limits<double>::lowest());
    RunTest(std::numeric_limits<double>::min());
    RunTest(std::numeric_limits<double>::epsilon());
    RunTest(0.634728);
    RunTest(-650.634728);
    RunTest(std::numeric_limits<double>::max());
    RunTest(std::numeric_limits<double>::infinity());

    RunTest(std::string{ "foo" });
    RunTest(true);
    RunTest(false);
    RunTest(TestJsonHelpersEnumClass::First);
    RunTest(TestJsonHelpersEnumClass::Middle);
    RunTest(TestJsonHelpersEnumClass::Last);
    RunTest(TestJsonHelpersEnum::First);
    RunTest(TestJsonHelpersEnum::Middle);
    RunTest(TestJsonHelpersEnum::Last);
}

TEST_CASE("Containers", "[json]")
{
    RunTest(std::vector{ 432, 65, 13456, -7542457, 754, 0 });
    RunTest(std::vector{ First, Last, Middle });
    RunTest(std::vector{ std::vector{ 0, 1, 2 }, std::vector{ 1, 0, 2 }, std::vector{ 2, 1, 0 } });
    RunTest(std::map<int, bool>{ { 93, true }, { -43, false }, { 42, true } });
    RunTest(std::set<int>{ 1, 2, 3, 44 });
}

TEST_CASE("Shared_Ptr", "[json]")
{
    RunTest(std::make_shared<int>(69));
    RunTest(std::make_shared<bool>(false));
    RunTest(std::make_shared<std::string>("foo"));
    RunTest(std::make_shared<std::vector<int>>(42, 44));
}

TEST_CASE("Pair", "[json]")
{
    RunTest(std::make_pair(69, true));
    RunTest(std::make_pair(std::string("bar"), TestJsonHelpersEnumClass::Last));
    RunTest(std::make_pair(0.4637289, std::make_pair(true, 44)));
}
