#ifndef JSONHELPERS_H
#define JSONHELPERS_H

#include "Concepts.h"

#include <nlohmann/json.hpp>

namespace Tril {

template <typename T>
class JsonSerialisationHelper;

template <typename T>
class JsonPolymorphicSerialisationHelper;

} // end namespace Tril

class JsonHelpers {
public:

/**
 * Can return true even if "target != toMatch" when target is a more inclusive
 * arithmetic type, i.e. when target is float, toMatch can be an int or unsigned
 */
static bool MatchType(nlohmann::json::value_t target, nlohmann::json::value_t toMatch);

/**
 * Ensures the supplied json is an object that contains exactly the specified
 * keys, of the correct types
 */
static bool ValidateJsonObject(const nlohmann::json& object, std::initializer_list<std::pair<std::string, nlohmann::json::value_t>>&& expectedEntries);

/**
 * Ensures the supplied array contains only the specified value type. For multi-
 * dimensional arrays, the dimensions are checked to ensure they are all the
 * same size.
 */
static bool ValidateJsonArray(const nlohmann::json& array, nlohmann::json::value_t valueType, unsigned dimensions = 1);

template <typename T>
static nlohmann::json Serialise(const T& value)
{
    constexpr bool isBasic = std::is_arithmetic_v<T> || std::is_same_v<T, std::string>;
    constexpr bool isEnum = std::is_enum_v<T>;
    constexpr bool isVector = IsVector<T>;
    constexpr bool isPair = IsPair<T>;
    constexpr bool isMap = IsMap<T>;
    constexpr bool isSharedPointerType = IsSharedPointer<T>;
    constexpr bool isJsonSerialisableType = IsJsonSerialisable<T>;
    constexpr bool isJsonPolymorphicallySerialisableType = IsJsonPolymorphicallySerialisable<T>;

    if constexpr (isBasic) {
        nlohmann::json serialised;
        serialised = value;
        return serialised;
    } else if constexpr (isEnum) {
        nlohmann::json serialised;
        serialised = static_cast<std::underlying_type_t<T>>(value);
        return serialised;
    } else if constexpr (isVector) {
        nlohmann::json serialised = nlohmann::json::array();
        for (auto& item : value) {
            serialised.push_back(Serialise(item));
        }
        return serialised;
    } else if constexpr (isPair) {
        nlohmann::json pair;
        pair["1"] = Serialise(value.first);
        pair["2"] = Serialise(value.second);
        return pair;
    } else if constexpr (isMap) {
        std::vector<typename T::value_type> items;
        for (const auto& item : value) {
            items.push_back(item);
        }
        return Serialise<decltype(items)>(items);
    } else if constexpr (isSharedPointerType) {
        return Serialise<typename T::element_type>(*value.get());
    } else if constexpr (isJsonSerialisableType) {
        return Tril::JsonSerialisationHelper<T>::Serialise(value);
    } else if constexpr (isJsonPolymorphicallySerialisableType ) {
        return Tril::JsonPolymorphicSerialisationHelper<T>::Serialise(value);
    } else {
        static_assert(isBasic || isEnum || isVector || isPair || isMap || isSharedPointerType || isJsonSerialisableType || isJsonPolymorphicallySerialisableType, "Type cannot be serialised");
        return {};
    }
}

template <typename T>
static T Deserialise(const nlohmann::json& serialised)
{
    constexpr bool isBasic = std::is_arithmetic_v<T> || std::is_same_v<T, std::string>;
    constexpr bool isEnum = std::is_enum_v<T>;
    constexpr bool isVector = IsVector<T>;
    constexpr bool isPair = IsPair<T>;
    constexpr bool isMap = IsMap<T>;
    constexpr bool isSharedPointerType = IsSharedPointer<T>;
    constexpr bool isJsonSerialisableType = IsJsonSerialisable<T>;
    constexpr bool isJsonPolymorphicallySerialisableType = IsJsonPolymorphicallySerialisable<T>;

    if constexpr (isBasic) {
        return serialised.get<T>();
    } else if constexpr (isEnum) {
        return static_cast<T>(serialised.get<std::underlying_type_t<T>>());
    } else if constexpr (isVector) {
        std::vector<typename T::value_type> items;
        for (const auto& item : serialised) {
            items.push_back(Deserialise<typename T::value_type>(item));
        }
        return items;
    } else if constexpr (isPair) {
        // FIXME repeating this code in Serialise, Deserialise and Validate is what JsonSerialisationHelper was all about preventing...
        return T{ Deserialise<typename T::first_type>(serialised.at("1")), Deserialise<typename T::second_type>(serialised.at("2")) };
    } else if constexpr (isSharedPointerType) {
        if constexpr (IsJsonSerialisable<typename T::element_type>) {
            return Tril::JsonSerialisationHelper<typename T::element_type>::DeserialiseShared(serialised);
        } else if constexpr (IsJsonPolymorphicallySerialisable<typename T::element_type>) {
            return Tril::JsonPolymorphicSerialisationHelper<typename T::element_type>::Deserialise(serialised);
        } else {
            return std::make_shared<typename T::element_type>(Deserialise<typename T::element_type>(serialised));
        }
    } else if constexpr (isMap) {
        T map;
        std::vector<typename T::value_type> items = Deserialise<decltype(items)>(serialised);
        for (const auto& item : items) {
            map.insert(item);
        }
        return map;
    } else if constexpr (isJsonSerialisableType) {
        return Tril::JsonSerialisationHelper<T>::Deserialise(serialised);
    } else if constexpr (isJsonPolymorphicallySerialisableType ) {
        static_assert(!isJsonPolymorphicallySerialisableType, "This type is a polymorphic base type, need to be returning a pointer to avoid slicing!");
        return Tril::JsonPolymorphicSerialisationHelper<T>::Deserialise(serialised);
    } else {
        static_assert(isBasic || isEnum || isVector || isPair || isMap || isSharedPointerType || isJsonSerialisableType || isJsonPolymorphicallySerialisableType, "Type cannot be deserialised");
        return {};
    }
}

template <typename T>
static bool Validate(const nlohmann::json& serialised)
{
    constexpr bool isBasic = std::is_arithmetic_v<T> || std::is_same_v<T, std::string>;
    constexpr bool isEnum = std::is_enum_v<T>;
    constexpr bool isVector = IsVector<T>;
    constexpr bool isPair = IsPair<T>;
    constexpr bool isMap = IsMap<T>;
    constexpr bool isSharedPointerType = IsSharedPointer<T>;
    constexpr bool isJsonSerialisableType = IsJsonSerialisable<T>;
    constexpr bool isJsonPolymorphicallySerialisableType = IsJsonPolymorphicallySerialisable<T>;

    if constexpr (isBasic || isEnum) {
        return MatchType(serialised.type(), GetStorageType<T>());
    } else if constexpr (isVector) {
        return MatchType(serialised.type(), GetStorageType<T>()) && ValidateJsonArray(serialised, GetStorageType<typename T::value_type>());
    } else if constexpr (isPair) {
        // FIXME repeating this code in Serialise, Deserialise and Validate is what JsonSerialisationHelper was all about preventing...
        return ValidateJsonObject(serialised, { {"1", GetStorageType<typename T::first_type>()}, {"2", GetStorageType<typename T::second_type>()} });
    } else if constexpr (isMap) {
        return Validate<std::vector<typename T::value_type>>(serialised);
    } else if constexpr (isSharedPointerType) {
        return Validate<typename T::element_type>(serialised);
    } else if constexpr (isJsonSerialisableType) {
        return MatchType(serialised.type(), GetStorageType<T>()) && Tril::JsonSerialisationHelper<T>::Validate(serialised);
    } else if constexpr (isJsonPolymorphicallySerialisableType ) {
        return Tril::JsonPolymorphicSerialisationHelper<T>::Validate(serialised);
    } else {
        static_assert(isBasic || isEnum || isVector || isPair || isMap || isSharedPointerType || isJsonSerialisableType || isJsonPolymorphicallySerialisableType, "Type cannot be serialised");
        return {};
    }
}

/**
 * Returns the type that would be used to store the Json value
 */
template <typename T>
static nlohmann::json::value_t GetStorageType()
{
    if constexpr (std::is_same_v<T, bool>) {
        return nlohmann::json::value_t::boolean;
    } else if constexpr (std::is_unsigned_v<T>) {
        return nlohmann::json::value_t::number_unsigned;
    } else if constexpr (std::is_integral_v<T>) {
        return nlohmann::json::value_t::number_integer;
    } else if constexpr (std::is_floating_point_v<T>) {
        return nlohmann::json::value_t::number_float;
    } else if constexpr (std::is_same_v<T, std::string>) {
        return nlohmann::json::value_t::string;
    } else if constexpr (std::is_enum_v<T>) {
        return GetStorageType<std::underlying_type_t<T>>();
    } else if constexpr (IsVector<T> || IsMap<T>) {
        return nlohmann::json::value_t::array;
    } else if constexpr (IsJsonSerialisable<T> || IsJsonPolymorphicallySerialisable<T> || IsPair<T>) {
        return nlohmann::json::value_t::object;
    } else if constexpr (IsSharedPointer<T>) {
        return GetStorageType<typename T::element_type>();
    }

    // Couldn't find a representation
    return nlohmann::json::value_t::null;
}

};

#endif // JSONHELPERS_H
