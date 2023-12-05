#include <SpatialMap.h>

#include <Shape.h>
#include <Random.h>

#include <catch2/catch.hpp>

using namespace util;

namespace {

class TestType {
public:
    constexpr static double RADIUS = 5.0;

    TestType(const Point& location, double bearing, double speed)
        : location_(location)
        , collide_{ location.x, location.y, RADIUS }
        , bearing_(bearing)
        , speed_(speed)
        , exists_(true)
    {
    }

    static std::shared_ptr<TestType> Random()
    {
        Point startingLoc{ Random::Number(-1000.0, 1000.0), Random::Number(-1000.0, 1000.0) };
        double bearing = Random::Number(0.0, std::numbers::pi * 2.0);
        double speed = Random::Boolean() ? 0.0 : Random::Number(0.0, 10.0);
        return std::make_shared<TestType>(startingLoc, bearing, speed);
    }

    const Point& GetLocation() const
    {
        return location_;
    }

    const Circle& GetCollide() const
    {
        return collide_;
    }

    bool Exists() const
    {
        return exists_;
    }

    bool Move()
    {
        if (speed_ != 0) {
            location_ = ApplyOffset(location_, bearing_, speed_);
            return true;
        }
        return false;
    }

    void Terminate()
    {
        exists_ = false;
    }

private:
    Point location_;
    Circle collide_;
    double bearing_;
    double speed_;
    bool exists_;
};


} // end anon namespace

TEST_CASE("SpatialMap", "[container]")
{
    Random::Seed(872346548);

    constexpr double regionSize = 100;
    constexpr double regionArea = regionSize * regionSize;
    SpatialMap<TestType> map(TestType::RADIUS, regionSize);
    REQUIRE(map.Size() == 0);

    // All following tests require that Size works correctly

    SECTION("Insert")
    {
        for (size_t i = 0; i < 123; ++i) {
            REQUIRE(map.Size() == i);
            map.Insert(TestType::Random());
            REQUIRE(map.Size() == i + 1);
        }
    }

    SECTION("Erase")
    {
        auto item = TestType::Random();
        size_t count = 15;
        for (size_t i = 0; i < count; ++i) {
            REQUIRE(map.Size() == i);
            map.Insert(item);
            REQUIRE(map.Size() == i + 1);
            map.Insert(TestType::Random());
            REQUIRE(map.Size() == i + 2);
            map.Erase(item);
            REQUIRE(map.Size() == i + 1);
        }
        REQUIRE(map.Size() == count);
    }

    SECTION("Clear")
    {
        for (size_t i = 0; i < 123; ++i) {
            map.Clear();
            REQUIRE(map.Size() == 0);
            size_t count = Random::Number<size_t>(5, 15);
            for (size_t j = 0; j < count; ++j) {
                map.Insert(TestType::Random());
            }
            REQUIRE(map.Size() == count);
        }
    }


    SECTION("Regions (Iterator)")
    {
        const size_t count = 123;
        for (size_t i = 0; i < count; ++i) {
            map.Insert(TestType::Random());
        }

        size_t regionCount = 0;
        for (const Rect& region : map.Regions()) {
            ++regionCount;
            REQUIRE_THAT(GetArea(region), Catch::Matchers::WithinAbs(regionArea, 0.0000000000001));
        }
        REQUIRE(regionCount > 0);
        REQUIRE(regionCount <= count);
        REQUIRE(regionCount == map.RegionCount());

        SECTION("Regions(Rect regionFilter)")
        {
            size_t filteredRegionCount = 0;
            for (const Rect& region : map.Regions(BoundingRect(Circle(0, 0, 500)))) {
                ++filteredRegionCount;
                REQUIRE_THAT(GetArea(region), Catch::Matchers::WithinAbs(regionArea, 0.0000000000001));
            }
            REQUIRE(filteredRegionCount > 0);
            REQUIRE(filteredRegionCount < regionCount);
        }

        map.Clear();

        regionCount = 0;
        for (const Rect& region : map.Regions()) {
            ++regionCount;
            REQUIRE_THAT(GetArea(region), Catch::Matchers::WithinAbs(regionArea, 0.0000000000001));
        }
        REQUIRE(regionCount == 0);
        REQUIRE(regionCount == map.RegionCount());

        SECTION("One region per item")
        {
            size_t expectedRegionCount = 0;
            for (int x = -10; x < 10; ++x) {
                for (int y = -10; y < 10; ++y) {
                    Point location{ (x * regionSize) + 3, (y * regionSize) + 3 };
                    map.Insert(std::make_shared<TestType>(location, 0.0, 0.0));
                    ++expectedRegionCount;
                    regionCount = 0;
                    for ([[maybe_unused]] const Rect& region : map.Regions()) {
                        ++regionCount;
                    }
                    REQUIRE(regionCount == expectedRegionCount);
                    REQUIRE(regionCount == map.RegionCount());
                }
            }
        }
    }

    SECTION("Items (Iterator)")
    {
        const size_t count = 123;
        for (size_t i = 0; i < count; ++i) {
            map.Insert(TestType::Random());
        }
        size_t counted = 0;
        for (const auto& item : map.Items()) {
            REQUIRE(item);
            REQUIRE(item->Exists());
            ++counted;
        }
        REQUIRE(counted == count);

        SECTION("Items(Rect regionFilter)")
        {
            size_t filteredItemCount = 0;
            for (const auto& item : map.Items(BoundingRect(Circle(0, 0, 500)))) {
                REQUIRE(item);
                REQUIRE(item->Exists());
                ++filteredItemCount;
            }
            REQUIRE(filteredItemCount > 0);
            REQUIRE(filteredItemCount < counted);

            const int repeats = 25;
            for (int i = 0; i < repeats; ++i) {
                map.Clear();
                const size_t items = Random::Number(7, 55);
                const Circle c{Random::Number<double>(-1000, 1000), Random::Number<double>(-1000, 1000), Random::Number<double>(100, 1000)};
                for (size_t i = 0; i < items; ++i) {
                    Point p = Random::PointIn(c);
                    REQUIRE(Contains(c, p));
                    REQUIRE(Collides(c, p));
                    map.Insert(std::make_shared<TestType>(p, 0, 0));
                }
                REQUIRE(map.Size() == items);

                size_t count = 0;
                for ([[ maybe_unused ]] auto& item : map.Items(BoundingRect(c))) {
                    ++count;
                }
                REQUIRE(count == items);

                count = 0;
                for ([[ maybe_unused ]] auto& item : map.ItemsCollidingWith(c)) {
                    ++count;
                }
                REQUIRE(count == items);

                for (int i = 0; i < 123; ++i) {
                    Rect rect{-100000, -100000, 10000, 10000};
                    for ([[ maybe_unused ]] auto& item : map.ItemsCollidingWith(Random::PointIn(rect))) {
                        ++count;
                    }
                }
            }
        }

        map.Clear();

        counted = 0;
        for (const auto& item : map.Items()) {
            REQUIRE(item);
            REQUIRE(item->Exists());
            ++counted;
        }
        REQUIRE(counted == 0);

        SECTION("Insert whilte Iterating")
        {
            REQUIRE(map.Size() == 0);
            for (int i = 0; i < 10; ++i) {
                map.Insert(TestType::Random());
            }
            REQUIRE(map.Size() == 10);
            size_t count = 0;
            for ([[ maybe_unused ]] const auto& item : map.Items()) {
                map.Insert(TestType::Random());
                ++count;
                REQUIRE(map.Size() == 10 + count);
            }
            REQUIRE(map.Size() == 20);
            count = 0;
            for ([[ maybe_unused ]] const auto& item : map.Items()) {
                map.Insert(TestType::Random());
                ++count;
                REQUIRE(map.Size() == 20 + count);
            }
            REQUIRE(map.Size() == 40);
            count = 0;
            for ([[ maybe_unused ]] const auto& item : map.Items(BoundingRect(Circle(0, 0, 500)))) {
                map.Insert(TestType::Random());
                ++count;
                REQUIRE(map.Size() == 40 + count);
            }
            REQUIRE(count > 0);
            REQUIRE(map.Size() == 40 + count);
        }
    }

    SECTION("ItemsCollidingWith (Iterator)")
    {
        const size_t count = 123;
        for (size_t i = 0; i < count; ++i) {
            map.Insert(TestType::Random());
        }
        size_t counted = 0;
        for (const auto& item : map.ItemsCollidingWith(Circle{ 0.0, 0.0, 1000.0 })) {
            REQUIRE(item);
            REQUIRE(item->Exists());
            ++counted;
        }
        REQUIRE(counted < count);

        map.Clear();

        counted = 0;
        for (const auto& item : map.Items()) {
            REQUIRE(item);
            REQUIRE(item->Exists());
            ++counted;
        }
        REQUIRE(counted == 0);
    }

    SECTION("RemoveIf")
    {
        const size_t initialCount = 123;
        for (size_t i = 0; i < initialCount; ++i) {
            map.Insert(TestType::Random());
        }
        REQUIRE(map.Size() == initialCount);

        map.RemoveIf([](const TestType& item) -> bool
        {
            return item.GetLocation().x < 0;
        });

        size_t newCount = map.Size();
        REQUIRE(newCount < initialCount);

        // same condition
        map.RemoveIf([](const TestType& item) -> bool
                     {
                         return item.GetLocation().x < 0;
                     });
        // Expect no change
        REQUIRE(newCount == map.Size());

        // new condition
        map.RemoveIf([](const TestType& item) -> bool
                     {
                         return item.GetLocation().y < 0;
                     });
        REQUIRE(map.Size() < newCount);

        for (const auto& item : map.Items()) {
            REQUIRE(item->GetLocation().x > 0.0);
            REQUIRE(item->GetLocation().y > 0.0);
        }
    }

    SECTION("MoveAndRemove")
    {
        const size_t initialCount = 123;
        for (size_t i = 0; i < initialCount; ++i) {
            REQUIRE(map.Size() == i);
            map.Insert(TestType::Random());
            REQUIRE(map.Size() == i + 1);
            map.MoveAndRemove();
            REQUIRE(map.Size() == i + 1);
        }

        REQUIRE(map.Size() == initialCount);
        REQUIRE(map.RegionCount() <= initialCount);

        const size_t regionCount = map.RegionCount();
        const size_t terminateCount = 50;
        size_t terminated = 0;
        while (terminated < terminateCount) {
            for(auto& item : map.Items()) {
                if (Random::Boolean() && terminated < terminateCount){
                    item->Terminate();
                    ++terminated;
                }
            }
        }
        // loop to prove that normal iteration doesn't remove items
        for ([[ maybe_unused ]] const auto& item : map.Items()) {}
        REQUIRE(map.RegionCount() == regionCount);
        REQUIRE(map.Size() == initialCount);
        map.MoveAndRemove();
        REQUIRE(map.RegionCount() < regionCount);
        REQUIRE(map.Size() == initialCount - terminateCount);

        map.Clear();
        // Not so much a test as a crash detector...
        map.MoveAndRemove();
    }

    SECTION("ItemsOnRegionBoundaries")
    {
        size_t items = 0;
        for (int x = -10; x < 10; ++x) {
            for (int y = -10; y < 10; ++y) {
                Point location{ x * regionSize, y * regionSize };
                map.Insert(std::make_shared<TestType>(location, 0.0, 100.0));
                ++items;
            }
        }

        for (int i = 0; i < 25; ++i) {
            map.MoveAndRemove();
            REQUIRE(map.Size() == items);
        }
    }
}

TEST_CASE("const SpatialMap", "[container]")
{
    Random::Seed(872346548);

    constexpr double regionSize = 100;
    constexpr double regionArea = regionSize * regionSize;
    SpatialMap<TestType> nonConstMap(TestType::RADIUS, regionSize);
    const SpatialMap<TestType>& map = nonConstMap;
    REQUIRE(map.Size() == 0);

    SECTION("Regions (Iterator)")
    {
        const size_t count = 123;
        for (size_t i = 0; i < count; ++i) {
            nonConstMap.Insert(TestType::Random());
        }

        size_t regionCount = 0;
        for (const Rect& region : map.Regions()) {
            ++regionCount;
            REQUIRE_THAT(GetArea(region), Catch::Matchers::WithinAbs(regionArea, 0.0000000000001));
        }
        REQUIRE(regionCount > 0);
        REQUIRE(regionCount <= count);
        REQUIRE(regionCount == map.RegionCount());

        SECTION("Regions(Rect regionFilter)")
        {
            size_t filteredRegionCount = 0;
            for (const Rect& region : map.Regions(BoundingRect(Circle(0, 0, 500)))) {
                ++filteredRegionCount;
                REQUIRE_THAT(GetArea(region), Catch::Matchers::WithinAbs(regionArea, 0.0000000000001));
            }
            REQUIRE(filteredRegionCount > 0);
            REQUIRE(filteredRegionCount < regionCount);
        }

        nonConstMap.Clear();

        regionCount = 0;
        for (const Rect& region : map.Regions()) {
            ++regionCount;
            REQUIRE_THAT(GetArea(region), Catch::Matchers::WithinAbs(regionArea, 0.0000000000001));
        }
        REQUIRE(regionCount == 0);
        REQUIRE(regionCount == map.RegionCount());

        SECTION("One region per item")
        {
            size_t expectedRegionCount = 0;
            for (int x = -10; x < 10; ++x) {
                for (int y = -10; y < 10; ++y) {
                    Point location{ (x * regionSize) + 3, (y * regionSize) + 3 };
                    nonConstMap.Insert(std::make_shared<TestType>(location, 0.0, 0.0));
                    ++expectedRegionCount;
                    regionCount = 0;
                    for ([[maybe_unused]] const Rect& region : map.Regions()) {
                        ++regionCount;
                    }
                    REQUIRE(regionCount == expectedRegionCount);
                    REQUIRE(regionCount == map.RegionCount());
                }
            }
        }
    }

    SECTION("Items (Iterator)")
    {
        const size_t count = 123;
        for (size_t i = 0; i < count; ++i) {
            nonConstMap.Insert(TestType::Random());
        }
        size_t counted = 0;
        for (const auto& item : map.Items()) {
            REQUIRE(item.Exists());
            ++counted;
        }
        REQUIRE(counted == count);

        SECTION("Items(Rect regionFilter)")
        {
            size_t filteredItemCount = 0;
            for (const auto& item : map.Items(BoundingRect(Circle(0, 0, 500)))) {
                REQUIRE(item.Exists());
                ++filteredItemCount;
            }
            REQUIRE(filteredItemCount > 0);
            REQUIRE(filteredItemCount < counted);

            // FIXME and repeat the tests here from non-const tests!
        }

        nonConstMap.Clear();

        counted = 0;
        for (const auto& item : map.Items()) {
            REQUIRE(item.Exists());
            ++counted;
        }
        REQUIRE(counted == 0);
    }

    SECTION("ItemsCollidingWith (Iterator)")
    {
        const size_t count = 123;
        for (size_t i = 0; i < count; ++i) {
            nonConstMap.Insert(TestType::Random());
        }
        size_t counted = 0;
        for (const auto& item : map.ItemsCollidingWith(Circle{ 0.0, 0.0, 1000.0 })) {
            REQUIRE(item.Exists());
            ++counted;
        }
        REQUIRE(counted < count);

        nonConstMap.Clear();

        counted = 0;
        for (const auto& item : map.Items()) {
            REQUIRE(item.Exists());
            ++counted;
        }
        REQUIRE(counted == 0);
    }
}

TEST_CASE("SpatialMap const iterators", "[container]")
{
    Random::Seed(872346548);

    constexpr double regionSize = 100;
    constexpr double regionArea = regionSize * regionSize;
    SpatialMap<TestType> map(TestType::RADIUS, regionSize);
    REQUIRE(map.Size() == 0);

    SECTION("Regions (Iterator)")
    {
        const size_t count = 123;
        for (size_t i = 0; i < count; ++i) {
            map.Insert(TestType::Random());
        }

        size_t regionCount = 0;
        for (const Rect& region : map.CRegions()) {
            ++regionCount;
            REQUIRE_THAT(GetArea(region), Catch::Matchers::WithinAbs(regionArea, 0.0000000000001));
        }
        REQUIRE(regionCount > 0);
        REQUIRE(regionCount <= count);
        REQUIRE(regionCount == map.RegionCount());

        SECTION("Regions(Rect regionFilter)")
        {
            size_t filteredRegionCount = 0;
            for (const Rect& region : map.CRegions(BoundingRect(Circle(0, 0, 500)))) {
                ++filteredRegionCount;
                REQUIRE_THAT(GetArea(region), Catch::Matchers::WithinAbs(regionArea, 0.0000000000001));
            }
            REQUIRE(filteredRegionCount > 0);
            REQUIRE(filteredRegionCount < regionCount);
        }

        map.Clear();

        regionCount = 0;
        for (const Rect& region : map.CRegions()) {
            ++regionCount;
            REQUIRE_THAT(GetArea(region), Catch::Matchers::WithinAbs(regionArea, 0.0000000000001));
        }
        REQUIRE(regionCount == 0);
        REQUIRE(regionCount == map.RegionCount());

        SECTION("One region per item")
        {
            size_t expectedRegionCount = 0;
            for (int x = -10; x < 10; ++x) {
                for (int y = -10; y < 10; ++y) {
                    Point location{ (x * regionSize) + 3, (y * regionSize) + 3 };
                    map.Insert(std::make_shared<TestType>(location, 0.0, 0.0));
                    ++expectedRegionCount;
                    regionCount = 0;
                    for ([[maybe_unused]] const Rect& region : map.CRegions()) {
                        ++regionCount;
                    }
                    REQUIRE(regionCount == expectedRegionCount);
                    REQUIRE(regionCount == map.RegionCount());
                }
            }
        }
    }

    SECTION("Items (Iterator)")
    {
        const size_t count = 123;
        for (size_t i = 0; i < count; ++i) {
            map.Insert(TestType::Random());
        }
        size_t counted = 0;
        for (const auto& item : map.CItems()) {
            REQUIRE(item.Exists());
            ++counted;
        }
        REQUIRE(counted == count);

        SECTION("Items(Rect regionFilter)")
        {
            size_t filteredItemCount = 0;
            for (const auto& item : map.CItems(BoundingRect(Circle(0, 0, 500)))) {
                REQUIRE(item.Exists());
                ++filteredItemCount;
            }
            REQUIRE(filteredItemCount > 0);
            REQUIRE(filteredItemCount < counted);

            // FIXME and repeat the tests here from non-const tests!
        }

        map.Clear();

        counted = 0;
        for (const auto& item : map.CItems()) {
            REQUIRE(item.Exists());
            ++counted;
        }
        REQUIRE(counted == 0);
    }

    SECTION("ItemsCollidingWith (Iterator)")
    {
        const size_t count = 123;
        for (size_t i = 0; i < count; ++i) {
            map.Insert(TestType::Random());
        }
        size_t counted = 0;
        for (const auto& item : map.CItemsCollidingWith(Circle{ 0.0, 0.0, 1000.0 })) {
            REQUIRE(item.Exists());
            ++counted;
        }
        REQUIRE(counted < count);

        map.Clear();

        counted = 0;
        for (const auto& item : map.CItems()) {
            REQUIRE(item.Exists());
            ++counted;
        }
        REQUIRE(counted == 0);
    }
}
