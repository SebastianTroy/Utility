#ifndef AUTOCLEARINGCONTAINER_H
#define AUTOCLEARINGCONTAINER_H

#include <memory>
#include <map>
#include <functional>

namespace util {

/**
 * A container that provides a handle per item contained, and lazily removes the
 * item once no remaining instances of the handle exist.
 */
using Handle = std::shared_ptr<int>;

template <typename ValueType>
class AutoClearingContainer {
public:

    using value_type = ValueType;

    [[nodiscard]] Handle PushBack(ValueType&& value)
    {
        auto lifetime = std::make_shared<int>(0);
        values_.insert({lifetime, std::move(value)});
        return lifetime;
    }

    void ForEach(const std::function<void(ValueType&)>& action)
    {
        std::erase_if(values_, [&](const auto& iter)
        {
            auto& [handle, value] = iter;
            auto handleValid = handle.lock();
            return !handleValid;
        });

        for (auto& pair : values_) {
            auto& [handle, value] = pair;
            auto handleValid = handle.lock();
            if (handleValid) {
                action(value);
            }
        }
    }

private:
    std::map<std::weak_ptr<int>, ValueType, std::owner_less<std::weak_ptr<int>>> values_;
};

} // end namespace util

#endif // AUTOCLEARINGCONTAINER_H
