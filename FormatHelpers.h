#ifndef FORMATHELPERS_H
#define FORMATHELPERS_H

#include "Shape.h"
#include "Transform.h"

#include <nlohmann/json.hpp>

#include <fmt/core.h>
#include <fmt/format.h>

#include <iostream>
#include <vector>

///**
// * fmt helper for printing anything iterable containing formattable items
// */
//template<Iterable Container>
//struct fmt::formatter<Container> : fmt::formatter<typename Container::value_type>
//{
//    template <typename FormatContext>
//    auto format(const Container& container, FormatContext& context)
//    {
//        auto&& out= context.out();
//        format_to(out, "{{");

//        bool first = true;
//        for (const auto& item : container) {
//            if (!first) {
//                first = false;
//            } else {
//                format_to(out, ", ");
//            }
//            fmt::formatter<typename Container::value_type>::format(item, context);
//        }

//        return format_to(out, "}}");
//    }
//};

/**
 * fmt helper for printing std::vectors containing formattable items
 */
template<typename ValueType>
struct fmt::formatter<std::vector<ValueType>> : fmt::formatter<ValueType>
{
    template <typename FormatContext>
    auto format(const std::vector<ValueType>& container, FormatContext& context)
    {
        auto&& out= context.out();
        format_to(out, "{{");

        bool first = true;
        for (const auto& item : container) {
            if (!first) {
                first = false;
            } else {
                format_to(out, ", ");
            }
            fmt::formatter<ValueType>::format(item, context);
        }

        return format_to(out, "}}");
    }
};

/**
 * iostream helper for printing std::pairs containing formattable items
 */
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& ostr, const std::pair<T1, T2>& pair)
{
    return ostr << "{" << pair.first << ", " << pair.second << "}";
}

/**
 * iostream helper for printing std::maps containing formattable keys and values
 */
template<typename KeyType, typename MappedType>
std::ostream& operator<<(std::ostream& ostr, const std::map<KeyType, MappedType>& map)
{
        ostr << "{";

        bool first = true;
        for (const auto& item : map) {
            if (!first) {
                first = false;
            } else {
                ostr << ", ";
            }
            ostr << item;
        }

        return ostr << "}";
}

/**
 * iostream helper for printing std::vectors containing formattable items
 */
template<typename ValueType>
std::ostream& operator<<(std::ostream& ostr, const std::vector<ValueType>& values)
{
        ostr << "{";

        bool first = true;
        for (const auto& item : values) {
            if (!first) {
                first = false;
            } else {
                ostr << ", ";
            }
            ostr << item;
        }

        return ostr << "}";
}

/**
 * iostream helper for printing util::Transform
 */
inline std::ostream& operator<<(std::ostream& ostr, const Transform& t)
{
    ostr << "Transform{ ";
    for (const auto& val : t.GetValues()) {
        ostr << val << ", ";
    }
    return ostr << " }";
}

/**
 * iostream helper for printing util::Circle
 */
inline std::ostream& operator<<(std::ostream& ostr, const Circle& c)
{
    return ostr << "Circle{ {x: " << c.x << ", y: " << c.y << "}, r: " << c.radius << "}";
}


/**
 * fmt helper for printing the type of variable stored inside nlohmann::json
 * values
 */
template<>
struct fmt::formatter<nlohmann::json::value_t>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const nlohmann::json::value_t& valueType, FormatContext& context)
    {
        auto&& out= context.out();

        switch (valueType) {
        case nlohmann::json::value_t::array:
            return format_to(out,"array");
        case nlohmann::json::value_t::binary:
            return format_to(out,"binary");
        case nlohmann::json::value_t::boolean:
            return format_to(out,"bool");
        case nlohmann::json::value_t::discarded:
            return format_to(out,"discarded");
        case nlohmann::json::value_t::null:
            return format_to(out,"null");
        case nlohmann::json::value_t::number_float:
            return format_to(out,"float");
        case nlohmann::json::value_t::number_integer:
            return format_to(out,"int");
        case nlohmann::json::value_t::number_unsigned:
            return format_to(out,"unsigned");
        case nlohmann::json::value_t::object:
            return format_to(out,"object");
        case nlohmann::json::value_t::string:
            return format_to(out,"string");
        }

        return format_to(out, "<INVALID JSON::VALUE_T VALUE>");
    }
};

#endif // FORMATHELPERS_H
