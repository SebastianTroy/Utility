#ifndef JSONPOLYMORPHICSERIALISATIONHELPER_H
#define JSONPOLYMORPHICSERIALISATIONHELPER_H

#include <JsonSerialisationHelper.h>

#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <map>

namespace util {

/**
 * This type allows polymorphic classes to be easily serialised, deserialised,
 * and validated.
 *
 * The aim here is to avoid massive code duplication of the Serialise,
 * Deserialise and Validate functions, both between functions in the same Type
 * and between different types that implement those functions.
 *
 * To make a type support serialisation in this way, you need to implementthe
 * following public functions:
 *
 *    virtual std::string TypeName() const
 *    {
 *        return std::string(util::TypeName<T>());
 *    }
 *
 *    static void ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<T>& helper)
 *    {
 *        helper.RegisterVariable("UniqueKey_1", &T::memberVariable1_);
 *        helper.RegisterVariable("UniqueKey_2", &T::memberVariable2_);
 *        ...
 *    }
 *
 * If T is not default constructable, then a constructor will need to be
 * registered as well as the variables. This can be necessary for initialising
 * const member variables, or for polymorphic types where the parent type does
 * not support default construction. Variables used for construction will be
 * automatically be registered for serialisation and deserialisation, and
 * therefore don't also need to be registered using RegisterVariable.
 *
 * Where our constructor is `T(ParameterType_1 p1, ParameterType_2 p2, ...)` we
 * would need to register the following:
 *
 * helper.RegisterConstructor(
 *            helper.CreateParameter<ParameterType_1>("UniqueKey_3", &T::memberVariable3_),
 *            helper.CreateParameter<ParameterType_2>("UniqueKey_4", [](constT& instance) -> ParameterType_2 { instance.GetValue(); }),
 *            ...
 *        );
 *
 * If a type has all been correctly registered, the following should be true:
 *
 * T instance; // initialised and containing state
 * T copy = JsonSerialisationHelper::Deserialise(JsonSerialisationHelper::Serialise(instance));
 * bool success = instance == copy; // should be true
 *
 *
 * Example of use:
 * ---------------
 *
 * class Animal {
 * public:
 *     virtual ~Animal() {}
 *     virtual std::string TypeName() const = 0;
 * };
 *
 * class Cat {
 * public:
 *     Cat(Colour furColour) : furColour(furColour) {}
 *     virtual ~Cat() {}
 *     std::string TypeName() const override { return std::string(util::TypeName<Cat>()); }
 *     static void ConfigureJsonSerialisationHelper(util::JsonSerialisationHelper<Cat>& helper)
 *     {
 *         helper.RegisterConstructor(
 *                    helper.CreateParameter<Colour>("Col", &T::furColour),
 *                );
 *         helper.RegisterVariable("Age", &T::age);
 *     }
 *
 * private:
 *     Colour furColour;
 *     int age;
 * };
 *
 * util::JsonPolymorphicSerialisationHelper<Animal>::template RegisterChildType<Cat>();
 * // Other types can and should be registerd up front too
 * util::JsonPolymorphicSerialisationHelper<Animal>::template RegisterChildType<Dog>();
 * util::JsonPolymorphicSerialisationHelper<Animal>::template RegisterChildType<Pangolin>();
 *
 * Animal* c = new Cat(furColour);
 * Animal* d = new Dog(breed);
 * Animal* p = new Pangolin(scaleCount);
 *
 * nlohmann::json serialised = JsonPolymorphicSerialisationHelper<Animal>::Serialise(*c);
 * std::shared_ptr<Cat> = JsonPolymorphicSerialisationHelper<Animal>::Deserialise(serialised);
 */
template <typename BaseType>
class JsonPolymorphicSerialisationHelper {
private:
    /**
     * An instance of this struct will be created for every registered child
     * type.
     */
    struct PolymorphicHelper {
    public:
        std::function<std::shared_ptr<BaseType>(const nlohmann::json& serialised)> deserialiser_ = nullptr;
        std::function<bool(const nlohmann::json& serialisedVariable)> validator_ = nullptr;
        std::function<nlohmann::json(const BaseType&)> serialiser_ = nullptr;
    };

public:

    [[nodiscard]] static std::shared_ptr<BaseType> Deserialise(const nlohmann::json& toDeserialise)
    {
        CheckType();
        CheckForTypeName(toDeserialise);
        if (toDeserialise.contains(TYPENAME_KEY) && toDeserialise.at(TYPENAME_KEY).is_string()) {
            PolymorphicHelper helper = GetInstance().childTypes_.at(toDeserialise.at(TYPENAME_KEY).get<std::string>());
            return helper.deserialiser_(toDeserialise);
        }
        return nullptr;
    }

    [[nodiscard]] static nlohmann::json Serialise(const BaseType& toSerialise)
    {
        CheckType();
        PolymorphicHelper helper = GetInstance().childTypes_.at(toSerialise.TypeName());
        return helper.serialiser_(toSerialise);
    }

    /**
     * @brief Validate Checks the supplied json object and ensures that it
     *                 contains all of the values required to fully initialise
     *                 an instance of type T.
     * @param json A JSON object, should have been createded using Serialise.
     * @return true if the json object can safely be Deserialised.
     */
    [[nodiscard]] static bool Validate(const nlohmann::json& json)
    {
        CheckType();
        CheckForTypeName(json);
        PolymorphicHelper helper = GetInstance().childTypes_.at(json.at(TYPENAME_KEY).get<std::string>());
        return helper.validator_(json);
    }

    template <typename ChildType>
    static void RegisterChildType()
    {
        static_assert(std::is_base_of_v<BaseType, ChildType>);
        static_assert(std::is_same_v<decltype(&BaseType::TypeName), std::string(BaseType::*)() const>, "BaseType must implement a 'std::string TypeName() const' function returning 'util::TypeName<T>();'");
        static_assert(std::is_same_v<decltype(&ChildType::TypeName), std::string(ChildType::*)() const>, "ChildType must implement a 'std::string TypeName() const override' function returning 'util::TypeName<T>();'");

        // Make sure the parent type can serialise / deserialise this type
        PolymorphicHelper polymorphicHelper;
        polymorphicHelper.deserialiser_ = [](const nlohmann::json& serialised) -> std::shared_ptr<BaseType>
        {
            return std::static_pointer_cast<BaseType>(JsonSerialisationHelper<ChildType>::DeserialiseShared(serialised));
        };
        polymorphicHelper.validator_ = [](const nlohmann::json& serialised) -> bool
        {
            bool valid = true;
            if (!serialised.contains(TYPENAME_KEY)) {
                valid = false;
                fmt::print("Missing '__typename' specifier in polymorphic serialised {}, {}", std::string(TypeName<ChildType>()), serialised.dump());
            }
            if (!serialised.at(TYPENAME_KEY).is_string()) {
                valid = false;
                fmt::print("Invalid '__typename' type in polymorphic serialised {}, require a string, got {}", std::string(TypeName<ChildType>()), serialised.at(TYPENAME_KEY).type());
            }
            if (serialised.at(TYPENAME_KEY).get<std::string>() != std::string(TypeName<ChildType>())) {
                valid = false;
                fmt::print("Invalid '__typename' value in serialised polymorphic type, expected {}, got {}", std::string(TypeName<ChildType>()), serialised.at(TYPENAME_KEY).get<std::string>());
            }
            auto copy = serialised;
            copy.erase(TYPENAME_KEY);
            return valid && JsonSerialisationHelper<ChildType>::Validate(copy);
        };
        polymorphicHelper.serialiser_ = [](const BaseType& instance) -> nlohmann::json
        {
            nlohmann::json serialised = JsonSerialisationHelper<ChildType>::Serialise(dynamic_cast<const ChildType&>(instance));
            serialised[TYPENAME_KEY] = std::string(TypeName<ChildType>());
            return serialised;
        };
        GetInstance().childTypes_.insert(std::make_pair(std::string(TypeName<ChildType>()), std::move(polymorphicHelper)));
    }

private:
    // Uses double underscore to avoid conflicting with user specified keys
    static inline std::string TYPENAME_KEY = "__typename";

    std::map<std::string, PolymorphicHelper> childTypes_;

    static JsonPolymorphicSerialisationHelper<BaseType>& GetInstance()
    {
        static JsonPolymorphicSerialisationHelper<BaseType> instance {};
        return instance;
    }

    static void CheckType()
    {
        if (GetInstance().childTypes_.empty()) {
            fmt::print("No child types registered for {}, perhaps you meant JsonSerialisationHelper<{}>?\n", std::string(util::TypeName<BaseType>()), std::string(util::TypeName<BaseType>()));
        }
    }

    static void CheckForTypeName(const nlohmann::json& json)
    {
        if (!json.contains(TYPENAME_KEY)) {
            fmt::print("Couldn't find \"__typename\" in {}, for type {}\n", json.dump(2), std::string(util::TypeName<BaseType>()));
        }
    }
};

} // end namespace util

#endif // JSONPOLYMORPHICSERIALISATIONHELPER_H
