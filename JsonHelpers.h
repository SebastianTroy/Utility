#ifndef JSONHELPERS_H
#define JSONHELPERS_H

#include "Concepts.h"

#include <nlohmann/json.hpp>

namespace util {

template <typename T>
class JsonSerialisationHelper;

template <typename T>
class JsonPolymorphicSerialisationHelper;

} // end namespace util

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
requires std::is_arithmetic_v<T> || std::is_same_v<T, std::string>
static nlohmann::json Serialise(const T& value)
{
    nlohmann::json serialised;
    serialised = value;
    return serialised;
}

template <typename T>
requires std::is_enum_v<T>
static nlohmann::json Serialise(const T& value)
{
    nlohmann::json serialised;
    serialised = static_cast<std::underlying_type_t<T>>(value);
    return serialised;
}

template <typename T>
requires std::ranges::range<T> && (!std::is_same_v<T, std::string>)
static nlohmann::json Serialise(const T& value)
{
    nlohmann::json serialised = nlohmann::json::array();
    for (auto& item : value) {
        serialised.push_back(Serialise(item));
    }
    return serialised;
}

template <typename T>
requires IsPair<T>
static nlohmann::json Serialise(const T& value)
{
    nlohmann::json pair;
    pair["1"] = Serialise(value.first);
    pair["2"] = Serialise(value.second);
    return pair;
}

template <typename T>
requires IsSharedPointer<T>
static nlohmann::json Serialise(const T& value)
{
    return Serialise<typename T::element_type>(*value.get());
}

template <typename T>
requires IsJsonSerialisable<T>
static nlohmann::json Serialise(const T& value)
{
    return util::JsonSerialisationHelper<T>::Serialise(value);
}

template <typename T>
requires IsJsonPolymorphicallySerialisable<T>
static nlohmann::json Serialise(const T& value)
{
    return util::JsonPolymorphicSerialisationHelper<T>::Serialise(value);
}

template <typename T>
requires std::is_arithmetic_v<T> || std::is_same_v<T, std::string>
static T Deserialise(const nlohmann::json& serialised)
{
    return serialised.get<T>();
}

template <typename T>
requires std::is_enum_v<T>
static T Deserialise(const nlohmann::json& serialised)
{
    return static_cast<T>(serialised.get<std::underlying_type_t<T>>());
}

template <typename T>
requires std::ranges::range<T> && (!std::is_same_v<T, std::string>) && requires (T c, typename T::value_type v) { c.insert(v); } && (!requires (T c, typename T::value_type v) { c.push_back(v); })
static T Deserialise(const nlohmann::json& serialised)
{
    T items;
    for (const auto& item : serialised) {
        items.insert(Deserialise<typename T::value_type>(item));
    }
    return items;
}

template <typename T>
requires std::ranges::range<T> && (!std::is_same_v<T, std::string>) && requires (T c, typename T::value_type v) { c.push_back(v); }
static T Deserialise(const nlohmann::json& serialised)
{
    T items;
    for (const auto& item : serialised) {
        items.push_back(Deserialise<typename T::value_type>(item));
    }
    return items;
}

template <typename T>
requires IsPair<T>
static T Deserialise(const nlohmann::json& serialised)
{
    // FIXME repeating this code in Serialise, Deserialise and Validate is what JsonSerialisationHelper was all about preventing...
    return T{ Deserialise<typename T::first_type>(serialised.at("1")), Deserialise<typename T::second_type>(serialised.at("2")) };
}

template <typename T>
requires IsSharedPointer<T>
static T Deserialise(const nlohmann::json& serialised)
{
    if constexpr (IsJsonSerialisable<typename T::element_type>) {
        return util::JsonSerialisationHelper<typename T::element_type>::DeserialiseShared(serialised);
    } else if constexpr (IsJsonPolymorphicallySerialisable<typename T::element_type>) {
        return util::JsonPolymorphicSerialisationHelper<typename T::element_type>::Deserialise(serialised);
    } else {
        return std::make_shared<typename T::element_type>(Deserialise<typename T::element_type>(serialised));
    }
}

template <typename T>
requires IsJsonSerialisable<T>
static T Deserialise(const nlohmann::json& serialised)
{
    return util::JsonSerialisationHelper<T>::Deserialise(serialised);
}

template <typename T>
requires IsJsonPolymorphicallySerialisable<T>
static T Deserialise(const nlohmann::json& serialised)
{
    // This function should never be used, instead the T type should be wrapped in a shared_ptr
    constexpr bool isJsonPolymorphicallySerialisableType = IsJsonPolymorphicallySerialisable<T>;
    static_assert(!isJsonPolymorphicallySerialisableType, "This type is a polymorphic base type, to avoid slicing use Deserialise(std::shared_ptr<T> ptr) instead!");
    return util::JsonPolymorphicSerialisationHelper<T>::Deserialise(serialised);
}

template <typename T>
requires std::is_arithmetic_v<T> || std::is_same_v<T, std::string> || std::is_enum_v<T>
static bool Validate(const nlohmann::json& serialised)
{
    return MatchType(serialised.type(), GetStorageType<T>());
}

template <typename T>
requires std::ranges::range<T> && (!std::is_same_v<T, std::string>)
static bool Validate(const nlohmann::json& serialised)
{
    return MatchType(serialised.type(), GetStorageType<T>()) && ValidateJsonArray(serialised, GetStorageType<typename T::value_type>());
}

template <typename T>
requires IsPair<T>
static bool Validate(const nlohmann::json& serialised)
{
    // FIXME repeating this code in Serialise, Deserialise and Validate is what JsonSerialisationHelper was all about preventing...
    return ValidateJsonObject(serialised, { {"1", GetStorageType<typename T::first_type>()}, {"2", GetStorageType<typename T::second_type>()} });
}

template <typename T>
requires IsSharedPointer<T>
static bool Validate(const nlohmann::json& serialised)
{
    return Validate<typename T::element_type>(serialised);
}

template <typename T>
requires IsJsonSerialisable<T>
static bool Validate(const nlohmann::json& serialised)
{
    return MatchType(serialised.type(), GetStorageType<T>()) && util::JsonSerialisationHelper<T>::Validate(serialised);
}

template <typename T>
requires IsJsonPolymorphicallySerialisable<T>
static bool Validate(const nlohmann::json& serialised)
{
    return util::JsonPolymorphicSerialisationHelper<T>::Validate(serialised);
}

/**
 * Returns the type that would be used to store the Json value
 */
template <typename T>
requires std::is_same_v<T, bool>
static nlohmann::json::value_t GetStorageType()
{
    return nlohmann::json::value_t::boolean;
}

template <typename T>
requires std::is_unsigned_v<T> && (!std::is_same_v<T, bool>)
static nlohmann::json::value_t GetStorageType()
{
    return nlohmann::json::value_t::number_unsigned;
}

template <typename T>
requires std::is_integral_v<T> && (!std::is_unsigned_v<T>) && (!std::is_same_v<T, bool>)
static nlohmann::json::value_t GetStorageType()
{
    return nlohmann::json::value_t::number_integer;
}

template <typename T>
requires std::is_floating_point_v<T>
static nlohmann::json::value_t GetStorageType()
{
    return nlohmann::json::value_t::number_float;
}

template <typename T>
requires std::is_same_v<T, std::string>
static nlohmann::json::value_t GetStorageType()
{
    return nlohmann::json::value_t::string;
}

template <typename T>
requires std::is_enum_v<T>
static nlohmann::json::value_t GetStorageType()
{
    return GetStorageType<std::underlying_type_t<T>>();
}

template <typename T>
requires std::ranges::range<T> && (!std::is_same_v<T, std::string>)
static nlohmann::json::value_t GetStorageType()
{
    return nlohmann::json::value_t::array;
}

template <typename T>
requires IsJsonSerialisable<T> || IsJsonPolymorphicallySerialisable<T> || IsPair<T>
static nlohmann::json::value_t GetStorageType()
{
    return nlohmann::json::value_t::object;
}

template <typename T>
requires IsSharedPointer<T>
static nlohmann::json::value_t GetStorageType()
{
    return GetStorageType<typename T::element_type>();
}

};

#endif // JSONHELPERS_H
