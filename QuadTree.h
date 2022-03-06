#ifndef QUADTREE_H
#define QUADTREE_H

#include "Shape.h"
#include "ChromeTracing.h"

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>
#include <concepts>

namespace util {

template <typename T>
concept QuadTreeCompatible = requires (T t) {
    { t.GetLocation() } -> std::same_as<const Point&>;
    { t.GetCollide() } -> Collidable;
};

/**
 * @brief Not really an iterator so much as a convinience class encapsulating
 * various iteration options and associated helpers.
 */
template <typename T>
requires QuadTreeCompatible<T>
class QuadTreeIterator {
public:
    QuadTreeIterator(std::function<void(const std::shared_ptr<T>& item)>&& action)
        : itemAction_(std::move(action))
        , quadFilter_([](const Rect&){ return true; })
        , itemFilter_([](const T&){ return true; })
        , removeItemPredicate_([](const T&){ return false; })
    {
    }

    QuadTreeIterator& SetQuadFilter(std::function<bool(const Rect& area)>&& filter)
    {
        quadFilter_ = std::move(filter);
        return *this;
    }
    template<Collidable C>
    QuadTreeIterator& SetQuadFilter(const C& c)
    {
        SetQuadFilter([=](const Rect& quadArea)
        {
            return Collides(c, quadArea);
        });
        return *this;
    }
    QuadTreeIterator& SetItemFilter(std::function<bool(const T& item)>&& filter)
    {
        itemFilter_ = std::move(filter);
        return *this;
    }
    template<Collidable C>
    QuadTreeIterator& SetItemFilter(const C& c)
    {
        SetItemFilter([=](const T& item)
        {
            TRACE_LAMBDA("ItemFilter<Collidable>")
            return Collides(c, item.GetCollide());
        });
        return *this;
    }
    QuadTreeIterator& SetRemoveItemPredicate(std::function<bool(const T& item)>&& removeItemPredicate)
    {
        removeItemPredicate_ = std::move(removeItemPredicate);
        return *this;
    }

    std::function<void(const std::shared_ptr<T>& item)> itemAction_;
    std::function<bool(const Rect& area)> quadFilter_;
    std::function<bool(const T& item)> itemFilter_;
    std::function<bool(const T& item)> removeItemPredicate_;
};

/**
 * @brief Not really an iterator so much as a convinience class encapsulating
 * various iteration options and associated helpers.
 */
template <typename T>
requires QuadTreeCompatible<T>
class ConstQuadTreeIterator {
public:
    ConstQuadTreeIterator(std::function<void(const T& item)>&& action)
        : itemAction_(std::move(action))
        , quadFilter_([](const Rect&){ return true; })
        , itemFilter_([](const T&){ return true; })
    {
    }

    ConstQuadTreeIterator& SetQuadFilter(std::function<bool(const Rect& area)>&& filter)
    {
        quadFilter_ = std::move(filter);
        return *this;
    }
    template<Collidable C>
    ConstQuadTreeIterator& SetQuadFilter(const C& c)
    {
        SetQuadFilter([=](const Rect& quadArea)
        {
            return Collides(c, quadArea);
        });
        return *this;
    }
    ConstQuadTreeIterator& SetItemFilter(std::function<bool(const T& item)>&& filter)
    {
        itemFilter_ = std::move(filter);
        return *this;
    }
    template<Collidable C>
    ConstQuadTreeIterator& SetItemFilter(const C& c)
    {
        SetItemFilter([=](const T& item)
        {
            return Collides(c, item.GetCollide());
        });
        return *this;
    }

    std::function<void(const T& item)> itemAction_;
    std::function<bool(const Rect& area)> quadFilter_;
    std::function<bool(const T& item)> itemFilter_;
};

template <typename T>
requires QuadTreeCompatible<T>
class QuadTree {
public:
    using Iter_t = QuadTreeIterator<T>;
    using ConstIter_t = ConstQuadTreeIterator<T>;

    QuadTree(const Rect& startArea, size_t itemCountTarget, size_t itemCountLeeway, double minQuadDiameter)
        : root_(std::make_shared<Quad>(nullptr, startArea))
        , rootExpandedCount_(0)
        , itemCountTarget_(std::max(itemCountTarget, size_t{1}))
        , itemCountLeeway_(std::min(itemCountTarget, itemCountLeeway))
        , minQuadDiameter_(minQuadDiameter)
        , currentlyIterating_(false)
    {
        TRACE_FUNC()
    }

    void Insert(std::shared_ptr<T> item)
    {
        TRACE_FUNC()
        AddItem(*root_, item, false);
    }
    void Clear()
    {
        TRACE_FUNC()
        assert(!currentlyIterating_);
        root_->children_ = std::nullopt;
        root_->items_.clear();
        root_->entering_.clear();
    }
    void RemoveIf(const std::function<bool(const T& item)>& predicate)
    {
        TRACE_FUNC()
        assert(!currentlyIterating_);
        bool requiresRebalance_ = false;
        ForEachQuad(*root_, [&](Quad& quad)
        {
            quad.items_.erase(std::remove_if(std::begin(quad.items_), std::end(quad.items_), [&](const auto& item) -> bool
            {
                return predicate(*item);
            }), std::end(quad.items_));

            requiresRebalance_ = requiresRebalance_ || static_cast<size_t>(std::abs(static_cast<int64_t>(itemCountTarget_)) - static_cast<int64_t>(quad.items_.size())) > itemCountLeeway_;
        });

        if (requiresRebalance_) {
            Rebalance();
        }
    }

    void ForEachQuad(const std::function<void(const Rect& area)>& action) const
    {
        TRACE_FUNC()
        ForEachQuad(*root_, [&](const Quad& quad)
        {
            action(quad.rect_);
        });
    }

    QuadTreeIterator<T> Iterator(std::function<void(const std::shared_ptr<T>& item)>&& action)
    {
        TRACE_FUNC()
        return QuadTreeIterator<T>(std::move(action));
    }

    ConstQuadTreeIterator<T> ConstIterator(std::function<void(const T& item)>&& action) const
    {
        TRACE_FUNC()
        return ConstQuadTreeIterator<T>(std::move(action));
    }

    /**
     * @brief Allows an action to be undertaken for each item in turn, however
     * not all items are included, only those contained within quads for which
     * quadFilter(quadArea) returns true.
     * @param action Performed for each item in the specified quads, in an
     * unspecified order.
     * @param quadFilter Each quad is tested based on this predicate, failed
     * quads will be skipped, as will their children.
     */
    void ForEachItem(const ConstQuadTreeIterator<T>& iter) const
    {
        TRACE_FUNC()
        ForEachQuad(*root_, [&](const Quad& quad)
        {
            for (const auto& item : quad.items_) {
                if (iter.itemFilter_(*item)) {
                    iter.itemAction_(*item);
                }
            }
        }, iter.quadFilter_);
    }

    /**
     * @brief ForEachItem Allows an action to be performed for each item that is
     * within a quad that passes the requirements of quadFilter. The
     * removeItemPredicate allows for the removal of unwanted items, it is
     * equivalent to calling RemoveIf with the same predicate.
     * @param iter This helper encapsulates a number of components, the action
     * to be performed for each item, an optional Quad filter that can be used
     * to cull quads for efficiency, an optional item filter that can be used to
     * select which items to apply the action to, and a removeItemPredicate,
     * which is equivalent to calling RemoveIf with the same predicate, but
     * wrapped up in a single pass.
     *
     * WARNING when using this function you MUST NOT change the result of
     * GetLocation() for any of the items, or the tree will stop working
     */
    void ForEachItemNoRebalance(const QuadTreeIterator<T>& iter) const
    {
        TRACE_FUNC()
        ForEachQuad(*root_, [&](const Quad& quad)
        {
            for (auto& item : quad.items_) {
                if (iter.itemFilter_(*item)) {
                    iter.itemAction_(item);
                }
            }
        }, iter.quadFilter_);
    }

    /**
     * @brief ForEachItem Allows an action to be performed for each item that is
     * within a quad that passes the requirements of quadFilter. As this is non-
     * const, item locations may change during this call, so a rebalance will
     * need to be performed after the call. Only one rebalance will occur, even
     * if this is called mid iteration (i.e. during an item's action). The
     * removeItemPredicate allows for the removal of unwanted items, it is
     * equivalent to calling RemoveIf with the same predicate.
     * @param iter This helper encapsulates a number of components, the action
     * to be performed for each item, an optional Quad filter that can be used
     * to cull quads for efficiency, an optional item filter that can be used to
     * select which items to apply the action to, and a removeItemPredicate,
     * which is equivalent to calling RemoveIf with the same predicate, but
     * wrapped up in a single pass.
     */
    void ForEachItem(const QuadTreeIterator<T>& iter)
    {
        TRACE_FUNC()
        bool wasIteratingAlready = currentlyIterating_;
        currentlyIterating_ = true;

        ForEachQuad(*root_, [&](const Quad& quad)
        {
            for (const auto& item : quad.items_) {
                if (iter.itemFilter_(*item)) {
                    iter.itemAction_(item);
                }
            }
        }, iter.quadFilter_);

        // Let the very first non-const iteration deal with all of the re-balancing
        if (!wasIteratingAlready) {
            currentlyIterating_ = false;

            ForEachQuad(*root_, [&](Quad& quad)
            {
                quad.items_.erase(std::remove_if(std::begin(quad.items_), std::end(quad.items_), [&](const auto& item) -> bool
                {
                    bool removeFromTree = iter.removeItemPredicate_(*item);
                    bool removeFromQuad = !Contains(quad.rect_, item->GetLocation());

                    if (!removeFromTree && removeFromQuad) {
                        AddItem(quad, item, true);
                    }

                    return removeFromTree || removeFromQuad;
                }), std::end(quad.items_));

                std::move(std::begin(quad.entering_), std::end(quad.entering_), std::back_inserter(quad.items_));
                quad.entering_.clear();
            });

            Rebalance();
        }
    }

    void SetItemCountTarget(unsigned target)
    {
        TRACE_FUNC()
        itemCountTarget_ = target;
    }
    void SetItemCountLeeway(unsigned leeway)
    {
        TRACE_FUNC()
        itemCountLeeway_ = leeway;
    }

    unsigned GetItemCountTaregt() const
    {
        TRACE_FUNC()
        return itemCountTarget_;
    }
    unsigned GetItemCountLeeway() const
    {
        TRACE_FUNC()
        return itemCountLeeway_;
    }
    size_t Size() const
    {
        TRACE_FUNC()
        return RecursiveItemCount(*root_);
    }

    /**
     * @brief Validate Used primarily for testing this container.
     */
    bool Validate() const
    {
        TRACE_FUNC()
        bool valid = true;

        // For easy breakpoint setting for debugging!
        auto Require = [&](bool val)
        {
            if (!val) {
                valid = false;
            }
        };

        // The following will not work if we are non-const looping, but we may want
        // to validate mid const loop, or make sure the const version is being called
        Require(!currentlyIterating_);

        ForEachQuad(*root_, [&](const Quad& quad) -> void
        {
            // Rect isn't too small
            double minDiameter = std::min(quad.rect_.right - quad.rect_.left, quad.rect_.bottom - quad.rect_.top);
            Require(minDiameter >= minQuadDiameter_);

            // Root quad has no parent
            if (&quad == root_.get()) {
                Require((quad.parent_ == nullptr));
            }

            if (quad.children_.has_value()) {
                // No items in quad containing chldren
                Require(quad.items_.empty());
                Require(quad.entering_.empty());

                // Having children implies at least one item stored within
                size_t count = 0;
                ForEachQuad(quad, [&](const Quad& quad)
                {
                    count += quad.items_.size();
                });
                Require(count > 0);

                for (const std::shared_ptr<Quad>& child : quad.children_.value()) {
                    // Child points at parent
                    Require(child->parent_ == &quad);
                }

                // Child rects are correct
                const Rect& parentRect = quad.rect_;
                double halfWidth = (parentRect.right - parentRect.left) / 2.0;
                double midX = parentRect.left + halfWidth;
                double midY = parentRect.top + halfWidth;

                Require(quad.children_.value().at(0)->rect_ == Rect{ parentRect.left, parentRect.top, midX            , midY              });
                Require(quad.children_.value().at(1)->rect_ == Rect{ midX           , parentRect.top, parentRect.right, midY              });
                Require(quad.children_.value().at(2)->rect_ == Rect{ parentRect.left, midY          , midX            , parentRect.bottom });
                Require(quad.children_.value().at(3)->rect_ == Rect{ midX           , midY          , parentRect.right, parentRect.bottom });
            } else {
                // Leaf quad should only have items_ in a const context
                Require(quad.entering_.empty());

                // All items should be within the bounds of the quad
                for (const auto& item : quad.items_) {
                    Require(Contains(quad.rect_, item->GetLocation()));
                }
            }
        });

        return valid;
    }

private:
    struct Quad {
        Quad* parent_;
        std::optional<std::array<std::shared_ptr<Quad>, 4>> children_;

        Rect rect_;
        std::vector<std::shared_ptr<T>> items_;
        std::vector<std::shared_ptr<T>> entering_;

        Quad(Quad* parent, Rect rect)
            : parent_(parent)
            , children_(std::nullopt)
            , rect_(rect)
            , items_{}
            , entering_{}
        {
        }
    };

    std::shared_ptr<Quad> root_;
    uint64_t rootExpandedCount_;
    size_t itemCountTarget_;
    size_t itemCountLeeway_;
    double minQuadDiameter_;
    bool currentlyIterating_;

    void ForEachQuad(Quad& quad, const std::function<void(Quad& quad)>& action)
    {
        TRACE_FUNC()
        ForEachQuad(quad, action, [](auto){ return true; });
    }
    void ForEachQuad(Quad& quad, const std::function<void(Quad& quad)>& action, const std::function<bool(const Rect&)>& filter)
    {
        TRACE_FUNC()
        action(quad);
        if (quad.children_.has_value()) {
            for (auto& child : quad.children_.value()) {
                if (filter(child->rect_)) {
                    ForEachQuad(*child, action, filter);
                }
            }
        }
    }
    void ForEachQuad(const Quad& quad, const std::function<void(const Quad& quad)>& action) const
    {
        TRACE_FUNC()
        ForEachQuad(quad, action, [](auto){ return true; });
    }
    void ForEachQuad(const Quad& quad, const std::function<void(const Quad& quad)>& action, const std::function<bool(const Rect&)>& filter) const
    {
        TRACE_FUNC()
        action(quad);
        if (quad.children_.has_value()) {
            for (const auto& child : quad.children_.value()) {
                if (filter(child->rect_)) {
                    ForEachQuad(*child, action, filter);
                }
            }
        }
    }

    void AddItem(Quad& startOfSearch, std::shared_ptr<T> item, bool preventRebalance)
    {
        TRACE_FUNC()
        if (currentlyIterating_) {
            QuadAt(startOfSearch, item->GetLocation()).entering_.push_back(item);
        } else {
            Quad& targetQuad = QuadAt(startOfSearch, item->GetLocation());
            targetQuad.items_.push_back(item);

            if (!preventRebalance) {
                Rebalance();
            }
        }
    }
    Quad& QuadAt(Quad& startOfSearch, const Point& location)
    {
        TRACE_FUNC()
        if (!Contains(startOfSearch.rect_, location)) {
            if (!startOfSearch.parent_) {
                ExpandRoot();
            }
            return QuadAt(*root_, location);
        } else if (startOfSearch.children_.has_value()) {
            size_t index = SubQuadIndex(startOfSearch.rect_, location);
            return QuadAt(*startOfSearch.children_.value().at(index), location);
        } else {
            return startOfSearch;
        }
    }

    void Rebalance()
    {
        TRACE_FUNC()
        assert(!currentlyIterating_);

        std::function<void(Quad& quad)> recursiveRebalance = [&](Quad& quad)
        {
            if (quad.children_.has_value()) {
                bool contract = true;
                size_t count = 0;
                for (auto& child : quad.children_.value()) {
                    recursiveRebalance(*child);
                    contract = contract && !child->children_.has_value();
                    count += child->items_.size();
                }
                if (contract && (count == 0 || count < itemCountTarget_ - itemCountLeeway_)) {
                    // Become a leaf quad if children contain too few entities
                    quad.items_ = RecursiveCollectItems(quad);
                    quad.children_ = std::nullopt;
                }
            } else if (quad.rect_.right - quad.rect_.left >= minQuadDiameter_ * 2.0 && quad.items_.size() > itemCountTarget_ + itemCountLeeway_) {
                // Lose leaf quad status if contains too many children UNLESS the new quads would be below the minimum size!
                quad.children_ = CreateChildren(quad);
                std::vector<std::shared_ptr<T>> itemsToRehome;
                itemsToRehome.swap(quad.items_);
                for (auto& item : itemsToRehome) {
                    QuadAt(quad, item->GetLocation()).items_.push_back(item);
                }
            }
        };

        recursiveRebalance(*root_);

        ContractRoot();
    }
    size_t RecursiveItemCount(const Quad& quad) const
    {
        TRACE_FUNC()
        size_t count = 0;
        ForEachQuad(quad, [&](const Quad& quad)
        {
            count += quad.items_.size();
        });
        return count;
    }
    std::vector<std::shared_ptr<T>> RecursiveCollectItems(Quad& quad)
    {
        TRACE_FUNC()
        std::vector<std::shared_ptr<T>> collectedItems;
        ForEachQuad(quad, [&](Quad& quad)
        {
            std::move(std::begin(quad.items_), std::end(quad.items_), std::back_inserter(collectedItems));
        });
        return collectedItems;
    }
    std::array<std::shared_ptr<Quad>, 4> CreateChildren(Quad& quad)
    {
        TRACE_FUNC()
        const Rect& parentRect = quad.rect_;

        double halfWidth = (parentRect.right - parentRect.left) / 2.0;
        double midX = parentRect.left + halfWidth;
        double midY = parentRect.top + halfWidth;

        return {
            std::make_shared<Quad>(&quad, Rect{ parentRect.left, parentRect.top, midX            , midY              }),
            std::make_shared<Quad>(&quad, Rect{ midX           , parentRect.top, parentRect.right, midY              }),
            std::make_shared<Quad>(&quad, Rect{ parentRect.left, midY          , midX            , parentRect.bottom }),
            std::make_shared<Quad>(&quad, Rect{ midX           , midY          , parentRect.right, parentRect.bottom }),
        };
    }

    void ExpandRoot()
    {
        TRACE_FUNC()
        bool expandOutwards = rootExpandedCount_++ % 2 == 0;
        const Rect& oldRootRect = root_->rect_;
        double width = oldRootRect.right - oldRootRect.left;
        double height = oldRootRect.bottom - oldRootRect.top;
        Rect newRootRect{
            oldRootRect.left - (expandOutwards ? 0.0 : width),
            oldRootRect.top - (expandOutwards ? 0.0 : height),
            oldRootRect.right + (expandOutwards ? width : 0.0),
            oldRootRect.bottom + (expandOutwards ? height : 0.0)
        };
        std::shared_ptr<Quad> oldRoot = root_;

        root_ = std::make_shared<Quad>(nullptr, newRootRect);
        oldRoot->parent_ = root_.get();
        root_->children_ = CreateChildren(*root_);
        root_->children_->at(expandOutwards ? 0 : 3).swap(oldRoot);
    }
    void ContractRoot()
    {
        TRACE_FUNC()
        if (root_->children_.has_value()) {
            unsigned count = 0;
            std::shared_ptr<Quad> quadWithItems;
            for (auto& child : root_->children_.value()) {
                if (child->items_.size() > 0 || child->children_.has_value()) {
                    ++count;
                    quadWithItems = child;
                }
            }
            if (count == 1) {
                root_ = quadWithItems;
                root_->parent_ = nullptr;
                --rootExpandedCount_;
            }
        }
    }

    size_t SubQuadIndex(const Rect& rect, const Point& p) const
    {
        TRACE_FUNC()
        //  ___
        // |0|1| Sub-Quad indices
        // |2|3|
        //  ---
        // Add one if in the right half (avoids branching)
        size_t lr = static_cast<size_t>(((p.x - rect.left) / (rect.right - rect.left)) + 0.5);
        // Add two if in the bottom half (avoids branching)
        size_t tb = static_cast<size_t>(((p.y - rect.top) / (rect.bottom - rect.top)) + 0.5) * 2;
        return lr + tb;
    }
};

} // namespace util

#endif // QUADTREE_H
