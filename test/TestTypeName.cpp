#include <TypeName.h>

#include <catch2/catch.hpp>

#include <string_view>
#include <map>

bool operator==(const std::string_view& a, const std::string_view& b)
{
    return a.compare(b) == 0;
}

std::ostream& operator<<(std::ostream& ostr, const std::string_view& sv)
{
    return ostr << std::string(sv);
}

using std::operator""sv;
using namespace util;

class TestClass {
public:
    class TestNestedClass {};
};
struct TestStruct {};
enum TestEnum {};
enum class TestEnumClassDefault {};
enum class TestEnumClassUnsignedLongLong : unsigned long long {};

namespace TestNs {
    class TestClass {
    public:
        class TestNestedClass {};
    };
    struct TestStruct {};
    enum TestEnum {};
    enum class TestEnumClassDefault {};
    enum class TestEnumClassUnsignedLongLong : unsigned long long {
    };
}

namespace {
    class AnonTestClass {
    public:
        class TestNestedClass {};
    };
    struct AnonTestStruct {};
    enum AnonTestEnum {};
    enum class AnonTestEnumClassDefault {};
    enum class AnonTestEnumClassUnsignedLongLong : unsigned long long {};
}

TEST_CASE("TypeName", "[Reflection]")
{
    SECTION("std types")
    {
        // More like std::__cxx11::basic_string<char>
        // REQUIRE(TypeName<std::string>() == "std::string"sv);
        REQUIRE(TypeName<std::vector<int>>() == "std::vector<int>"sv);
        REQUIRE(TypeName<std::map<int, bool>>() == "std::map<int, bool>"sv);
    }

    SECTION("Custom types")
    {
        REQUIRE(TypeName<TestStruct>() == "TestStruct"sv);
        REQUIRE(TypeName<TestClass>() == "TestClass"sv);
        REQUIRE(TypeName<TestClass::TestNestedClass>() == "TestClass::TestNestedClass"sv);
        REQUIRE(TypeName<TestEnum>() == "TestEnum"sv);
        REQUIRE(TypeName<TestEnumClassDefault>() == "TestEnumClassDefault"sv);
        REQUIRE(TypeName<TestEnumClassUnsignedLongLong>() == "TestEnumClassUnsignedLongLong"sv);
    }

    SECTION("Interaction with decltype")
    {
        REQUIRE(TypeName<decltype(TestStruct{})>() == "TestStruct"sv);
    }

    SECTION("Named namespace")
    {
        REQUIRE(TypeName<TestNs::TestStruct>() == "TestNs::TestStruct"sv);
        REQUIRE(TypeName<TestNs::TestClass>() == "TestNs::TestClass"sv);
        REQUIRE(TypeName<TestNs::TestClass::TestNestedClass>() == "TestNs::TestClass::TestNestedClass"sv);
        REQUIRE(TypeName<TestNs::TestEnum>() == "TestNs::TestEnum"sv);
        REQUIRE(TypeName<TestNs::TestEnumClassDefault>() == "TestNs::TestEnumClassDefault"sv);
        REQUIRE(TypeName<TestNs::TestEnumClassUnsignedLongLong>() == "TestNs::TestEnumClassUnsignedLongLong"sv);
    }

    SECTION("Anonymous namespace")
    {
        REQUIRE(TypeName<::AnonTestStruct>().ends_with("::AnonTestStruct"sv));
        REQUIRE(TypeName<::AnonTestClass>().ends_with("::AnonTestClass"sv));
        REQUIRE(TypeName<::AnonTestClass::TestNestedClass>().ends_with("::AnonTestClass::TestNestedClass"sv));
        REQUIRE(TypeName<::AnonTestEnum>().ends_with("::AnonTestEnum"sv));
        REQUIRE(TypeName<::AnonTestEnumClassDefault>().ends_with("::AnonTestEnumClassDefault"sv));
        REQUIRE(TypeName<::AnonTestEnumClassUnsignedLongLong>().ends_with("::AnonTestEnumClassUnsignedLongLong"sv));
    }
}
