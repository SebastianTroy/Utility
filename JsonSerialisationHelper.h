#ifndef JSONSERIALISATIONHELPER_H
#define JSONSERIALISATIONHELPER_H

#include "JsonHelpers.h"
#include "Concepts.h"
#include "TypeName.h"

#include <nlohmann/json.hpp>
#include <fmt/core.h>

#include <assert.h>

namespace Tril {

/**
 * This type allows classes to easily be serialised, deserialised and validated.
 *
 * The aim here is to avoid massive code duplication of the Serialise,
 * Deserialise and Validate functions, both between functions in the same Type
 * and between different types that implement those functions.
 *
 * To make a type support serialisation in this way, you need to implement a
 * single static function:
 *    static void ConfigureJsonSerialisationHelper(Tril::JsonSerialisationHelper<T>& helper)
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
 */
template <typename T>
class JsonSerialisationHelper {
private:
    /**
     * Helper to represent a value that needs to be saved/loaded and validated.
     */
    struct Variable {
    public:
        std::string key_;
        nlohmann::json::value_t storageType_;

        std::function<void(const T& source, nlohmann::json& target)> writer_;
        std::function<bool(const nlohmann::json& serialisedVariable)> validator_;
        std::function<void(const nlohmann::json& source, T& target)> parser_;
    };

    /**
     * Helper to represent a value that is needed to construct an instance of
     * the type T. It contains a 'Variable' to utilise the validate and write
     * functionality, but the deserialise functionality is replaced.
     */
    template <typename ParamType>
    struct Parameter {
    public:
        using value_t = ParamType;
        Variable value_;
    };

    /**
     * For non-trivially constructable types we need to be able to create an
     * instance when deserialising, in those cases we need this Constructor
     * type.
     */
    struct Constructor {
    public:
        std::function<T(const nlohmann::json& serialised)> parser_ = nullptr;
        std::function<std::shared_ptr<T>(const nlohmann::json& serialised)> parserToShared_ = nullptr;
    };

public:
    [[nodiscard]] static T Deserialise(const nlohmann::json& toDeserialise)
    {
        assert(Validate(toDeserialise));
        T deserialised = GetInstance().constructionHelper_.parser_(toDeserialise);
        for (const auto& [ key, memberHelper ] : GetInstance().memberVariables_) {
            memberHelper.parser_(toDeserialise.at(memberHelper.key_), deserialised);
        }
        return deserialised;
    }

    [[nodiscard]] static std::shared_ptr<T> DeserialiseShared(const nlohmann::json& toDeserialise)
    {
        assert(Validate(toDeserialise));
        std::shared_ptr<T> deserialised = GetInstance().constructionHelper_.parserToShared_(toDeserialise);
        for (const auto& [ key, memberHelper ] : GetInstance().memberVariables_) {
            memberHelper.parser_(toDeserialise.at(memberHelper.key_), *deserialised);
        }
        return deserialised;
    }

    [[nodiscard]] static nlohmann::json Serialise(const T& toSerialise)
    {
        nlohmann::json serialised = nlohmann::json::object();
        for (const auto& [ key, memberHelper ] : GetInstance().memberVariables_) {
            memberHelper.writer_(toSerialise, serialised);
        }
        return serialised;
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
        bool valid = json.is_object();
        if (valid) {
            for (const auto& [ key, jsonValue ] : json.items()) {
                if (GetInstance().memberVariables_.count(key) == 1) {
                    if (!JsonHelpers::MatchType(GetInstance().memberVariables_.at(key).storageType_, jsonValue.type())) {
                        fmt::print("Invalid storage Type for variable of type {} with key {}: Expected {}, got {}.\n", Tril::TypeName<T>(), key, GetInstance().memberVariables_.at(key).storageType_, jsonValue.type());
                        assert(false && "Invalid storage Type for variable");
                        valid = false;
                    } else if (!GetInstance().memberVariables_.at(key).validator_(jsonValue)) {
                        fmt::print("Invalid value for serialised variable with key: {}, & value: {}.\n", key, jsonValue.dump());
                        assert(false && "Invalid value for serialised variable");
                        valid = false;
                    }
                } else {
                    fmt::print("Invalid key found in object of type {}: {}.\n", Tril::TypeName<T>(), key);
                    assert(false && "Invalid key found in object");
                    valid = false;
                }
            }
        }
        return valid;
    }

    /**
     * Not all types can be default constructable. There may be const member
     * values that need to be instantiated upon construction, for example.
     *
     * The intended usage for this function is:
     *
     * helper.RegisterConstructor(
     *                            helper.CreateParameter(),
     *                            helper.CreateParameter(),
     *                            ...
     *                            );
     */
    template <typename... Parameters>
    void RegisterConstructor(Parameters... params)
    {
        static_assert((IsInstance<Parameters, Parameter> && ...), "All parameters must be of type Parameter<...>");

        if (constructionHelper_.parser_ != nullptr) {
            fmt::print("Cannot register multiple constructors for type {}", Tril::TypeName<T>());
            assert(false && "Cannot register multiple constructors for type in JsonSerialisationHelper<T>");
        }

        constructionHelper_.parser_ = [=](const nlohmann::json& serialised) -> T
        {
            return T((JsonHelpers::Deserialise<typename decltype(params)::value_t>(serialised.at(params.value_.key_)))...);
        };
        constructionHelper_.parserToShared_ = [=](const nlohmann::json& serialised) -> std::shared_ptr<T>
        {
            return std::make_shared<T>((JsonHelpers::Deserialise<typename decltype(params)::value_t>(serialised.at(params.value_.key_)))...);
        };
    }

    /**
     * Creates a helper that can parse a parameter from the serialised object
     * in order to construct the object, the value still needs to be saved to
     * the JSON for serialisation, in this overload a lamda is supplied that can
     * read the value from an instance of the class for serialisation.
     *
     * @param key - The key to use when storing the value in JSON
     * @param getter - The function used to get the value to store in JSON when
     * the value is serialised.
     */
    template <typename ValueType>
    [[nodiscard]] Parameter<ValueType> CreateParameter(const std::string& key, std::function<ValueType(const T& instance)>&& getter)
    {
        Variable helper {};
        helper.key_ = key;
        helper.storageType_ = JsonHelpers::GetStorageType<ValueType>();
        helper.writer_ = [key = helper.key_, getter = std::move(getter)](const T& source, nlohmann::json& target)
        {
            target[key] = JsonHelpers::Serialise<ValueType>(getter(source));
        };
        helper.validator_ = [storageType = helper.storageType_](const nlohmann::json& serialisedVariable) -> bool
        {
            return JsonHelpers::Validate<ValueType>(serialisedVariable);
        };
        helper.parser_ = [](const nlohmann::json& source, T& target)
        {
            // Do nothing, this value was handled during construction
        };
        AddVariable(std::move(helper));
        return Parameter<ValueType>{ helper };
    }
    /**
     * Creates a helper that can parse a parameter from the serialised object
     * in order to construct the object, the value still needs to be saved to
     * the JSON for serialisation, in this overload a member variable pointer is
     * required for extracting the value for serialisation.
     *
     * @param key - The key to use when storing the value in JSON
     * @param getter - The function used to get the value to store in JSON when
     * the value is serialised.
     */
    template <typename ParameterType>
    [[nodiscard]] Parameter<ParameterType> CreateParameter(const std::string& key, ParameterType T::* toRegister)
    {
        Variable helper {};
        helper.key_ = key;
        helper.storageType_ = JsonHelpers::GetStorageType<ParameterType>();
        helper.writer_ = [key = helper.key_, memberPtr = toRegister](const T& source, nlohmann::json& target)
        {
            target[key] = JsonHelpers::Serialise<ParameterType>(source.*memberPtr);
        };
        helper.validator_ = [storageType = helper.storageType_](const nlohmann::json& serialisedVariable) -> bool
        {
            return JsonHelpers::Validate<ParameterType>(serialisedVariable);
        };
        helper.parser_ = [memberPtr = toRegister](const nlohmann::json& source, T& target)
        {
            // Do nothing, this value was handled during construction
        };
        AddVariable(std::move(helper));
        return Parameter<ParameterType>{ helper };
    }

    /**
     * IMPORTANT The registered type MUST be move or copy assignable, If a type
     * cannot be copy be copy or move assignable, consider making the member
     * being registered into a unique or shared_ptr.
     *
     * NOTE the member pointed by toRegister to can be private!
     */
    template <typename VariableType>
    void RegisterVariable(const std::string& key, VariableType T::* toRegister)
    {
        static_assert(std::is_copy_assignable_v<VariableType> || std::is_move_assignable_v<VariableType>, "Registered type must be move or copy assignable");

        if (JsonHelpers::GetStorageType<VariableType>() == nlohmann::json::value_t::null) {
            fmt::print("Cannot register variable {}, of type {}, unknown storage type.\n", key, Tril::TypeName<VariableType>());
            assert(false && "Cannot register a variable with null storage type");
        }

        Variable helper {};

        helper.key_ = key;
        helper.storageType_ = JsonHelpers::GetStorageType<VariableType>();
        helper.writer_ = [key = helper.key_, memberPtr = toRegister](const T& source, nlohmann::json& target)
        {
            target[key] = JsonHelpers::Serialise<VariableType>(source.*memberPtr);
        };
        helper.validator_ = [storageType = helper.storageType_](const nlohmann::json& serialisedVariable) -> bool
        {
            return JsonHelpers::Validate<VariableType>(serialisedVariable);
        };
        helper.parser_ = [memberPtr = toRegister](const nlohmann::json& source, T& target)
        {
            target.*memberPtr = JsonHelpers::Deserialise<VariableType>(source);
        };

        AddVariable(std::move(helper));
    }

private:
    Constructor constructionHelper_;
    std::map<std::string, Variable> memberVariables_;

    /**
     * Really didn't want to use a singleton, BUT it was the only way to prevent
     * linking failure. Not sure entirely what was causing it, but statically
     * calling T::ConfigureJsonSerialisationHelper() during variable
     * initialisation was possibly the culpret...
     */
    static JsonSerialisationHelper<T>& GetInstance()
    {
        static JsonSerialisationHelper<T> helper {};
        return helper;
    }

    JsonSerialisationHelper()
        : constructionHelper_{}
        , memberVariables_{}
    {
          static_assert(std::is_same_v<decltype(&T::ConfigureJsonSerialisationHelper), void (*)(JsonSerialisationHelper<T>&)>);
          T::ConfigureJsonSerialisationHelper(*this);

        // Make sure our constructor is valid
        constexpr bool isTriviallyConstructable = std::is_default_constructible_v<T>;
        if constexpr (isTriviallyConstructable) {
            // The type may be trivially constructable, but also have
            if (constructionHelper_.parser_ == nullptr) {
                constructionHelper_.parser_ = [](const nlohmann::json&) -> T { return {}; };
                constructionHelper_.parserToShared_ = [](const nlohmann::json&) -> std::shared_ptr<T> { return std::make_shared<T>(); };
            }
        } else if constexpr (!isTriviallyConstructable) {
            if (constructionHelper_.parser_ == nullptr) {
                fmt::print("Must register a constructor for a non-default constructable type!: {}\n", Tril::TypeName<T>());
                assert(false && "JsonSerialisationHelper<T>: Must register a constructor for a non-default constructable type!");
            }
        }
    }

    void AddVariable(Variable&& helper)
    {
        if (memberVariables_.count(helper.key_) != 0) {
            fmt::print("Duplicate key registered for type {}, key: {}\n", Tril::TypeName<T>(), helper.key_);
        }
        memberVariables_.insert(std::make_pair(helper.key_, helper));
    }
};

} // end namespace Tril

#endif // JSONSERIALISATIONHELPER_H
