#ifndef TESTJSONSERIALISATIONHELPER_H
#define TESTJSONSERIALISATIONHELPER_H

#include <JsonSerialisationHelper.h>
#include <FormatHelpers.h>

#include <string>
#include <vector>

/*
 * Some Types for testing
 */

class TrivialTestType {
public:
    int a_;
    std::vector<int> b_;
    std::string c_;

    static void ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<TrivialTestType>& helper)
    {
        helper.RegisterVariable("TrivialTestType::a_", &TrivialTestType::a_);
        helper.RegisterVariable("TrivialTestType::b_", &TrivialTestType::b_);
        helper.RegisterVariable("TrivialTestType::c_", &TrivialTestType::c_);
    }

    bool operator==(const TrivialTestType& other) const
    {
        return (a_ == other.a_) && (b_ == other.b_) && (c_ == other.c_);
    }

    bool operator!=(const TrivialTestType& other) const
    {
        return !this->operator==(other);
    }
};

class NonTrivialTestType {
public:
    bool b_;

    NonTrivialTestType(const NonTrivialTestType& other) = default;
    NonTrivialTestType(NonTrivialTestType&& other) = default;

    NonTrivialTestType(int a)
        : a_(a)
    {
    }

    static void ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<NonTrivialTestType>& helper)
    {
        helper.RegisterConstructor(
                    helper.template CreateParameter<decltype(a_)>("NonTrivialTestType::a_", [](const NonTrivialTestType& instance) { return instance.GetA(); })
                    );
        helper.RegisterVariable("NonTrivialTestType::b_", &NonTrivialTestType::b_);
    }

    const int& GetA() const
    {
        return a_;
    }

    bool operator==(const NonTrivialTestType& other) const
    {
        return (a_ == other.a_) && (b_ == other.b_);
    }

    bool operator!=(const NonTrivialTestType& other) const
    {
        return !this->operator==(other);
    }

private:
    const int a_;
};

class NestedType {
public:
    TrivialTestType b_;

    NestedType(const NestedType& other) = default;
    NestedType(NestedType&& other) = default;

    NestedType(NonTrivialTestType a)
        : a_(a)
    {
    }

    static void ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<NonTrivialTestType>& helper)
    {
        helper.RegisterConstructor(
                    helper.template CreateParameter<decltype(a_)>("NestedType::a_", [](const NonTrivialTestType& instance) { return instance.GetA(); })
                    );
        helper.RegisterVariable("NestedType::b_", &NonTrivialTestType::b_);
    }

    const NonTrivialTestType& GetA() const
    {
        return a_;
    }

    bool operator==(const NestedType& other) const
    {
        return (a_ == other.a_) && (b_ == other.b_);
    }

    bool operator!=(const NestedType& other) const
    {
        return !this->operator==(other);
    }

private:
    NonTrivialTestType a_;
};

/*
 * Some operators to make testing easier
 */

std::ostream& operator<<(std::ostream& ostr, const TrivialTestType& value)
{
    return ostr << "TrivialTestType{" << value.a_ << ", " << value.b_ << ", " << value.c_ << "}";
}

std::ostream& operator<<(std::ostream& ostr, const NonTrivialTestType& value)
{
    return ostr << "NonTrivialTestType{" << value.GetA() << ", " << value.b_ << "}";
}


#endif // TESTJSONSERIALISATIONHELPER_H
