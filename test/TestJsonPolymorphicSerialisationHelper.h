#ifndef TESTJSONPOLYMORPHICSERIALISATIONHELPER_H
#define TESTJSONPOLYMORPHICSERIALISATIONHELPER_H

#include <TypeName.h>
#include <JsonPolymorphicSerialisationHelper.h>

#include <string>

/*
 * Some Types for testing
 */

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
        return std::string(util::TypeName<ChildTestTypeA>());
    }

    static void ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<ChildTestTypeA>& helper)
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

    bool operator==(const ChildTestTypeA& other) const
    {
        return num_ == other.num_ && GetFact() == other.GetFact();
    }

    bool operator!=(const ChildTestTypeA& other) const
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
        return std::string(util::TypeName<ChildTestTypeB>());
    }

    static void ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<ChildTestTypeB>& helper)
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

    bool operator==(const ChildTestTypeB& other) const
    {
        return num_ == other.num_ && GetFact() == other.GetFact();
    }

    bool operator!=(const ChildTestTypeB& other) const
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

class NestedType {
public:
    std::shared_ptr<BaseTestType> polymorphic_;

    static void ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<NestedType>& helper)
    {
        helper.RegisterVariable("NestedType::polymorphic_", &NestedType::polymorphic_);
    }
};

/*
 * Some operators to make testing easier
 */

std::ostream& operator<<(std::ostream& ostr, const ChildTestTypeA& value)
{
    return ostr << util::TypeName<ChildTestTypeA>() << "{" << value.GetNumber() << ", " << value.GetFact() << "}";
}

std::ostream& operator<<(std::ostream& ostr, const ChildTestTypeB& value)
{
    return ostr << util::TypeName<ChildTestTypeB>() << "{" << value.GetNumber() << ", " << value.GetFact() << "}";
}

#endif // TESTJSONPOLYMORPHICSERIALISATIONHELPER_H
