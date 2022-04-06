#include "JsonHelpers.h"

#include <fmt/core.h>

using namespace nlohmann;

bool JsonHelpers::MatchType(json::value_t target, json::value_t toMatch)
{
    return target == toMatch
            || (target == json::value_t::number_float && (toMatch == json::value_t::number_integer || toMatch == json::value_t::number_unsigned))
            || (target == json::value_t::number_integer && toMatch == json::value_t::number_unsigned);
}

bool JsonHelpers::ValidateJsonObject(const json& object, std::initializer_list<std::pair<std::string, json::value_t> >&& expectedEntries)
{
    bool valid = object.is_object() && object.size() == expectedEntries.size();

    if (valid) {
        for (const auto& [ key, requiredType ] : expectedEntries) {
            if (!object.contains(key) || !MatchType(object.at(key).type(), requiredType)) {
                valid = false;
                break;
            }
        }
    }
    return valid;
}

bool JsonHelpers::ValidateJsonArray(const json& array, json::value_t valueType, unsigned dimensions)
{
    bool valid = array.type() == json::value_t::array;

    if (valid){
        if (dimensions == 0) {
            valid = false;
        } else if (dimensions > 1) {
            // Make sure all children are arrays
            if ((valid = ValidateJsonArray(array, json::value_t::array, 1))) {
                // Make sure all children are the same size
                size_t subArraySize = array.empty() ? 0 : array.size();
                for (const auto& subArray : array) {
                    if (subArray.size() != subArraySize) {
                        valid = false;
                        break;
                    } else {
                        // Now recursively check all dimensions of the array
                        if (!ValidateJsonArray(subArray, valueType, dimensions - 1)) {
                            valid = false;
                            break;
                        }
                    }
                }
            }
        } else {
            for (const auto& item : array) {
                if (!MatchType(valueType, item.type())) {
                    valid = false;
                    break;
                }
            }
        }
    }

    return valid;
}


