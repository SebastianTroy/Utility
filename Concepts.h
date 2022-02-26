#ifndef CONCEPTS_H
#define CONCEPTS_H

#include <vector>
#include <map>
#include <memory>
#include <experimental/type_traits>

// FIXME use C++20 concepts when available

template<typename T, typename... Types>
constexpr inline bool IsAllSameType = (... && std::is_same<T, Types>::value);

// Useful to tell if typename T is an instance of a particular, usage e.g. IsInstance<T, std::vector>
template <typename Test, template<typename...> class Ref>
constexpr inline bool IsInstance = false;
template <template<typename...> class Ref, typename... Args>
constexpr inline bool IsInstance<Ref<Args...>, Ref> = true;

template<typename T>
constexpr inline bool IsVector = IsInstance<T, std::vector>;

template<typename T>
constexpr inline bool IsPair = IsInstance<T, std::pair>;

template<typename T>
constexpr inline bool IsMap = IsInstance<T, std::map>;

template <typename T>
constexpr inline bool IsSharedPointer = IsInstance<T, std::shared_ptr>;

template<typename T>
using JsonSerialisable_t = decltype( &T::ConfigureJsonSerialisationHelper );
template<typename T>
constexpr bool IsJsonSerialisable = std::experimental::is_detected_v<JsonSerialisable_t, T>;

template<typename T>
using JsonPolymorphicallySerialisable_t = decltype( &T::TypeName);
template<typename T>
constexpr bool IsJsonPolymorphicallySerialisable = std::experimental::is_detected_v<JsonPolymorphicallySerialisable_t, T>;

#endif // CONCEPTS_H
