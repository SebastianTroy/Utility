#ifndef SPATIALMAP_H
#define SPATIALMAP_H

#include "Shape.h"

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace util {

template <typename T>
concept SpatialMapCompatible = requires (T& t, const T& ct) {
    { ct.GetLocation() } -> std::same_as<const Point&>;
    { ct.GetCollide() } -> Collidable;
    { ct.Exists() } -> std::same_as<bool>;
    { t.Move() } -> std::same_as<bool>;
};

/**
 * @brief The SpatialMap class is meant to be an alternative to QuadTree.
 *
 * The aim was to create a flat heirarchy that might be more performant than
 * a QuadTree, and also simpler to create iterators for, allowing more natural
 * usage of the container.
 *
 * The iterator classes are not meant to be used directly, instead they are
 * intended to be used via the e.g. `Items()` and `Regions()` functions:
 *
 * ```c++
 *     for (auto& item : spatialMap.Items()) { ... };
 * ```
 *
 * Future work may include fleshing out the iterators to allow for stl algorithm
 * compatability. And perhaps const versions of the iterators.
 */
template <typename T>
    requires SpatialMapCompatible<T>
class SpatialMap {
private:
    struct Region;
    using MapType = std::unordered_map<uint64_t, Region>;
    using ContainerType = std::vector<std::shared_ptr<T>>;

public:
    ///
    /// ITERATOR HELPERS
    ///

    class RegionIteratorHelper {
    public:
        class RegionIterator {
        public:
            explicit RegionIterator(MapType::iterator&& iter)
                : regionIter_(std::move(iter))
            {
            }

            RegionIterator& operator++()
            {
                ++regionIter_;
                return *this;
            }

            RegionIterator& End()
            {
                regionIter_ = {};
                return *this;
            }

            Region& CurrentRegion()
            {
                return regionIter_->second;
            }

            bool operator!=(const RegionIterator& other) const
            {
                return regionIter_ != other.regionIter_;
            }

            const Rect& operator*() const
            {
                return regionIter_->second.area_;
            }

            const MapType::key_type& Key() const
            {
                return regionIter_->first;
            }

        private:
            MapType::iterator regionIter_;
        };

        using iterator = RegionIterator;
        using value_type = Rect;
        using size_type = size_t;

        RegionIteratorHelper(SpatialMap<T>& container)
            : container_(container)
        {
        }

        RegionIterator begin()
        {
            return RegionIterator(container_.regions_.begin());
        }

        RegionIterator end()
        {
            return RegionIterator(container_.regions_.end());
        }

    private:
        SpatialMap<T>& container_;
    };

    class FilteredRegionIteratorHelper {
    public:
        class RegionIterator {
        public:
            explicit RegionIterator(SpatialMap<T>& container, const Rect& regionFilter)
                : regionFilter_(regionFilter)
                , map_(container.regions_)
                , currentRegion_(nullptr)
            {
                std::tie(minX_, minY_) = container.GetCoordinate({ regionFilter.left, regionFilter.top });
                std::tie(maxX_, maxY_) = container.GetCoordinate({ regionFilter.right, regionFilter.bottom });
                x_ = minX_ - 1;
                y_ = minY_;
                Next();
            }

            RegionIterator& operator++()
            {
                Next();
                return *this;
            }

            RegionIterator& End()
            {
                currentRegion_ = nullptr;
                return *this;
            }

            bool operator!=(const RegionIterator& other) const
            {
                return currentRegion_ != other.currentRegion_;
            }

            const Rect& operator*() const
            {
                return currentRegion_->area_;
            }

            MapType::key_type Key() const
            {
                return GetCoordinateKey({x_, y_});
            }

            Region& CurrentRegion()
            {
                return *currentRegion_;
            }

        private:
            const Rect& regionFilter_;
            int32_t minX_, maxX_, minY_, maxY_;
            MapType& map_;
            Region* currentRegion_;
            int32_t x_, y_;

            void Next()
            {
                do {
                    ++x_;
                    if (x_ > maxX_) {
                        x_ = minX_;
                        ++y_;
                        if (y_ > maxY_) {
                            // Set state to end() iterator
                            currentRegion_ = nullptr;
                            return;
                        }
                    }
                    currentRegion_ = map_.count(Key()) > 0 ? &map_.at(Key()) : nullptr;
                } while (currentRegion_ == nullptr);
            }
        };

        using iterator = RegionIterator;
        using value_type = Rect;
        using size_type = size_t;

        FilteredRegionIteratorHelper(SpatialMap<T>& container, const Rect& regionFilter)
            : container_(container)
            , regionFilter_(regionFilter)
        {
        }

        RegionIterator begin()
        {
            return RegionIterator(container_, regionFilter_);
        }

        RegionIterator end()
        {
            return RegionIterator(container_, regionFilter_).End();
        }

    private:
        SpatialMap<T>& container_;
        const Rect& regionFilter_;
    };

    template <typename RegionIteratorHelperType>
    class ItemIteratorHelper {
    public:
        class ItemIterator {
        public:
            explicit ItemIterator(RegionIteratorHelperType& regionIteratorHelper)
                : regionIteratorHelper_(regionIteratorHelper)
                , regionIter_(regionIteratorHelper_.begin())
                , itemIter_{}
            {
                if (regionIter_ != std::end(regionIteratorHelper_)) {
                    itemIter_ = std::begin(regionIter_.CurrentRegion().items_);
                }
            }

            ItemIterator& operator++()
            {
                ++itemIter_;
                if (itemIter_ == std::end(regionIter_.CurrentRegion().items_)) {
                    ++regionIter_;
                    if (regionIter_ != std::end(regionIteratorHelper_)) {
                        itemIter_ = std::begin(regionIter_.CurrentRegion().items_);
                    } else {
                        itemIter_ = {};
                    }
                }
                return *this;
            }

            ItemIterator& End()
            {
                regionIter_.End();
                itemIter_ = {};
                return *this;
            }

            bool operator!=(const ItemIterator& other) const
            {
                return other.regionIter_ != regionIter_ && other.itemIter_ != itemIter_;
            }

            std::shared_ptr<T>& operator*()
            {
                return *itemIter_;
            }

        private:
            RegionIteratorHelperType regionIteratorHelper_;
            RegionIteratorHelperType::iterator regionIter_;
            ContainerType::iterator itemIter_;
        };

        using iterator = ItemIterator;
        using value_type = std::shared_ptr<T>;
        using size_type = size_t;

        ItemIteratorHelper(RegionIteratorHelperType&& regionIteratorHelper, SpatialMap<T>& container)
            : container_(container)
            , regionIteratorHelper_(std::move(regionIteratorHelper))
        {
            container_.OnBeginIteration();
        }

        ~ItemIteratorHelper()
        {
            container_.OnEndIteration();
        }

        ItemIterator begin()
        {
            return ItemIterator(regionIteratorHelper_);
        }

        ItemIterator end()
        {
            return ItemIterator(regionIteratorHelper_).End();
        }

    private:
        SpatialMap<T>& container_;
        RegionIteratorHelperType regionIteratorHelper_;
    };

    template <typename RegionIteratorHelperType, typename ColliderType>
    class FilteredItemIteratorHelper {
    public:
        class ItemIterator {
        public:
            explicit ItemIterator(RegionIteratorHelperType& regionIteratorHelper, const ColliderType& collider)
                : regionIteratorHelper_(regionIteratorHelper)
                , regionIter_(regionIteratorHelper_.begin())
                , itemIter_{}
                , nullIter_{}
                , collider_(collider)
            {
                if (regionIter_ != std::end(regionIteratorHelper_)) {
                    itemIter_ = std::begin(regionIter_.CurrentRegion().items_);
                    Next();
                }
            }

            ItemIterator& operator++()
            {
                Next();
                return *this;
            }

            ItemIterator& End()
            {
                regionIter_.End();
                itemIter_ = {};
                return *this;
            }

            bool operator!=(const ItemIterator& other) const
            {
                return other.regionIter_ != regionIter_ && other.itemIter_ != itemIter_;
            }

            std::shared_ptr<T>& operator*()
            {
                return *itemIter_;
            }

        private:
            RegionIteratorHelperType regionIteratorHelper_;
            RegionIteratorHelperType::iterator regionIter_;
            ContainerType::iterator itemIter_;
            ContainerType::iterator nullIter_;
            const ColliderType& collider_;

            void Next()
            {
                do {
                    ++itemIter_;
                    if (itemIter_ == std::end(regionIter_.CurrentRegion().items_)) {
                        ++regionIter_;
                        if (regionIter_ != std::end(regionIteratorHelper_)) {
                            itemIter_ = std::begin(regionIter_.CurrentRegion().items_);
                        } else {
                            itemIter_ = {};
                        }
                    }
                } while (itemIter_ != nullIter_ && !Collides(collider_, (*itemIter_)->GetCollide()));
            }
        };

        using iterator = ItemIterator;
        using value_type = std::shared_ptr<T>;
        using size_type = size_t;

        FilteredItemIteratorHelper(RegionIteratorHelperType&& regionIteratorHelper, SpatialMap<T>& container, const ColliderType& collider)
            : container_(container)
            , regionIteratorHelper_(std::move(regionIteratorHelper))
            , collider_(collider)
        {
            container_.OnBeginIteration();
        }

        ~FilteredItemIteratorHelper()
        {
            container_.OnEndIteration();
        }

        ItemIterator begin()
        {
            return ItemIterator(regionIteratorHelper_, collider_);
        }

        ItemIterator end()
        {
            return ItemIterator(regionIteratorHelper_, collider_).End();
        }

    private:
        SpatialMap<T>& container_;
        RegionIteratorHelperType regionIteratorHelper_;
        const ColliderType& collider_;
    };

    ///
    /// SpatialMap implementaion
    ///

    SpatialMap(double maxEntityRadius, double regionSize)
        : regions_{}
        , maxEntityRadius_(maxEntityRadius)
        , regionSize_(regionSize)
        , currentIterators_(0)
    {
    }

    RegionIteratorHelper Regions()
    {
        return RegionIteratorHelper(*this);
    }

    FilteredRegionIteratorHelper Regions(const Rect& regionFilter)
    {
        return FilteredRegionIteratorHelper(*this, regionFilter);
    }

    ItemIteratorHelper<RegionIteratorHelper> Items()
    {
        return ItemIteratorHelper(RegionIteratorHelper(*this), *this);
    }

    ItemIteratorHelper<FilteredRegionIteratorHelper> Items(const Rect& regionFilter)
    {
        // increase bounding rect size because entities might be in a neighboring region, but overlap into our filter area
        return ItemIteratorHelper(FilteredRegionIteratorHelper(*this, BoundingRect(regionFilter, maxEntityRadius_)), *this);
    }

    template <typename ColliderType>
        requires Collidable<ColliderType>
    FilteredItemIteratorHelper<FilteredRegionIteratorHelper, ColliderType> ItemsCollidingWith(ColliderType itemFilter)
    {
        return FilteredItemIteratorHelper(FilteredRegionIteratorHelper(*this, BoundingRect(itemFilter, maxEntityRadius_)), *this, itemFilter);
    }

    void Insert(std::shared_ptr<T> item)
    {
        AddItem(item);
    }

    void Erase(const std::shared_ptr<T>& toErase)
    {
        std::vector<std::shared_ptr<T>>& container = RegionAt(toErase->GetLocation()).items_;
        container.erase(std::remove_if(std::begin(container), std::end(container), [&](const auto& x) -> bool { return x.get() == toErase.get();}), std::end(container));
    }

    void Clear()
    {
        regions_.clear();
    }

    void RemoveIf(const std::function<bool(const T& item)>& predicate)
    {
        OnBeginIteration();
        std::erase_if(regions_, [&](auto& iter) -> bool
        {
            auto& [ key, region ] = iter;
            region.items_.erase(std::remove_if(std::begin(region.items_), std::end(region.items_), [&](const auto& item) -> bool
                {
                    return predicate(*item);
                }), std::end(region.items_));
            return region.items_.empty();
        });
        OnEndIteration();
    }

    void MoveAndRemove()
    {
        OnBeginIteration();

        std::erase_if(regions_, [&](auto& pair) -> bool
            {
                auto& [ key, region ] = pair;

                region.items_.erase(std::remove_if(std::begin(region.items_), std::end(region.items_), [&](auto& item) -> bool
                    {
                        bool removeItemCompletely = !item->Exists();
                        bool movedToDifferentRegion = !removeItemCompletely && item->Move() && GetCoordinate(item->GetLocation()) != region.coordinates_;

                        if (movedToDifferentRegion) {
                            AddItem(item);
                        }
                        return removeItemCompletely || movedToDifferentRegion;
                    }), std::end(region.items_));

                return region.items_.empty();
            });

        OnEndIteration();
    }

    size_t Size() const
    {
        size_t count = itemsAddedDuringIteration_.size();
        for (const auto& [ key, region ] : regions_) {
            count += region.items_.size();
        }
        return count;
    }

    size_t RegionCount() const
    {
        return regions_.size();
    }

private:
    struct Region {
        ContainerType items_ {};
        Rect area_;
        std::pair<int32_t, int32_t> coordinates_;

        Region(const Rect& area, const std::pair<int32_t, int32_t>& coordinates)
            : area_(area)
            , coordinates_(coordinates)
        {
        }
    };

    MapType regions_;

    double maxEntityRadius_;
    double regionSize_;

    // Intended to track recursive iteration, not multi-threaded iteration
    unsigned currentIterators_;
    std::vector<std::shared_ptr<T>> itemsAddedDuringIteration_;

    void OnBeginIteration()
    {
        ++currentIterators_;
    }

    void OnEndIteration()
    {
        if (--currentIterators_ == 0) {
            for (const auto& item : itemsAddedDuringIteration_) {
                AddItem(item);
            }
            itemsAddedDuringIteration_.clear();
        }
    }

    void AddItem(const std::shared_ptr<T>& item)
    {
        if (currentIterators_ != 0) {
            itemsAddedDuringIteration_.push_back(item);
        } else {
            RegionAt(item->GetLocation()).items_.push_back(item);
        }
    }

    Region& RegionAt(const Point& location)
    {
        auto coords = GetCoordinate(location);
        auto key = GetCoordinateKey(coords);
        if (!regions_.contains(key)) {
            double left = coords.first * regionSize_;
            double top = coords.second * regionSize_;
            double right =  (coords.first  + 1) * regionSize_;
            double bottom = (coords.second + 1) * regionSize_;
            regions_.insert(std::pair{ key, Region(Rect{ left, top, right, bottom }, coords) });
        }
        return regions_.at(key);
    }

    std::pair<int32_t, int32_t> GetCoordinate(const Point& location) const
    {
        return { static_cast<int32_t>((location.x / regionSize_) - std::signbit(location.x)),
                static_cast<int32_t>((location.y / regionSize_) - std::signbit(location.y))};
    }

    static uint64_t GetCoordinateKey(const std::pair<int32_t, int32_t>& coords)
    {
        uint64_t x = std::bit_cast<uint64_t>(static_cast<int64_t>(coords.first));
        uint64_t y = std::bit_cast<uint64_t>(static_cast<int64_t>(coords.second));
        return ((x & 0xFFFFFFFF) << 0)
               | ((y & 0xFFFFFFFF) << 32);
    }
};

} // namespace util

#endif // SPATIALMAP_H
