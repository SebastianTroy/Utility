#ifndef TYPENAME_H
#define TYPENAME_H

#include <string>
#include <string_view>
#include <sstream>

namespace util {

// credit https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/56766138#56766138
template <typename T>
[[ nodiscard ]] constexpr std::string_view TypeName()
{
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "std::string_view util::TypeName() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr std::string_view util::TypeName() [with T = ";
    suffix = "; std::string_view = std::basic_string_view<char>]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl util::TypeName<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

template<typename Arg, typename... Args>
std::string TypeNames(const std::string& seperator)
{
    std::stringstream result;
    result << TypeName<Arg>();
    if constexpr (sizeof...(Args) > 0) {
        result << seperator << TypeNames<Args...>(seperator);
    }
    return result.str();
}

} // end namespace util

#endif // TYPENAME_H
