#include <QuadTree.h>
#include <Random.h>

#include <catch2/catch.hpp>

using namespace Tril;

namespace {

class TestType {
public:
    Point location_;
    Circle collide_;

    TestType(const Point& location)
        : location_(location)
        , collide_{ location.x, location.y, 0 }
    {
    }

    const Point& GetLocation() const
    {
        return location_;
    }

    const Circle& GetCollide() const
    {
        return collide_;
    }
};

inline auto PointComparator = [](const Point& a, const Point& b)
{
    return a.x < b.x || (a.x == b.x && a.y < b.y);
};

}

TEST_CASE("QuadTree", "[container]")
{
    Random::Seed(42);

    SECTION("Empty Tree")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;
        QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);

        REQUIRE(tree.Validate());
    }

    SECTION("One item - in bounds")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;
        QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);

        Point testLoc{ 5.0, 5.0 };
        std::shared_ptr<TestType> testItem = std::make_shared<TestType>(testLoc);

        tree.Insert(testItem);

        REQUIRE(tree.Validate());
        REQUIRE(tree.Size() == 1);
        REQUIRE(testItem->GetLocation() == testLoc);

        size_t count = 0;
        tree.ForEachItem(ConstQuadTreeIterator<TestType>([&](const TestType& item)
        {
            ++count;
            REQUIRE(item.GetLocation() == testLoc);
        }));

        REQUIRE(count == 1);
    }

    SECTION("Multiple items - in bounds, unique locations")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;
        QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);

        size_t count = 25;
        std::vector<Point> testLocations;

        for (size_t i = 0; i < count; ++i) {
            testLocations.push_back(Random::PointIn(area));
            tree.Insert(std::make_shared<TestType>(testLocations.back()));
            REQUIRE(tree.Validate());
        }

        REQUIRE(tree.Size() == count);

        std::vector<Point> itemLocations;

        size_t itemCount = 0;
        tree.ForEachItem(ConstQuadTreeIterator<TestType>([&](const TestType& item)
        {
            ++itemCount;
            itemLocations.push_back(item.GetLocation());
        }));

        std::sort(std::begin(testLocations), std::end(testLocations), PointComparator);
        std::sort(std::begin(itemLocations), std::end(itemLocations), PointComparator);

        REQUIRE(itemLocations == testLocations);
        REQUIRE(itemCount == count);
    }

    SECTION("Multiple items - in bounds, same location")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;
        QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);

        size_t count = 25;
        Point testLoc = Random::PointIn(area);

        for (size_t i = 0; i < count; ++i) {
            tree.Insert(std::make_shared<TestType>(testLoc));
        }

        REQUIRE(tree.Validate());
        REQUIRE(tree.Size() == count);

        size_t itemCount = 0;
        tree.ForEachItem(ConstQuadTreeIterator<TestType>([&](const TestType& item)
        {
            ++itemCount;
            REQUIRE(item.GetLocation() == testLoc);
        }));

        REQUIRE(itemCount == count);
    }

    SECTION("Clear")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;
        QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);

        REQUIRE(tree.Validate());
        REQUIRE(tree.Size() == 0);
        tree.Clear();
        REQUIRE(tree.Validate());
        REQUIRE(tree.Size() == 0);

        for (int i = 0; i < 25; ++i) {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
            REQUIRE(tree.Validate());
            REQUIRE(tree.Size() == 1);
            tree.Clear();
            REQUIRE(tree.Validate());
            REQUIRE(tree.Size() == 0);
        }
    }

    SECTION("Items out of bounds")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;

        SECTION("Single item - oob")
        {
            auto outOfBoundsPoints = { Point{ -1, -1 }, Point{ -1, 1 }, Point{ 1, -1 }, // around top left
                                       Point{ 11, -1 }, Point{ 11, 1 }, Point{ 9, -1 }, // around top right
                                       Point{ 11, 11 }, Point{ 11, 9 }, Point{ 9, 11 }, // around bottom right
                                       Point{ -1, 11 }, Point{ -1, 9 }, Point{ 1, 11 }, // around bottom left
                                       Point{ -100, 11 }, Point{ -1, 100 }, Point{ 100, 110 }, // way out of bounds
                                     };
            for (const auto& testLoc : outOfBoundsPoints) {
                QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);
                tree.Insert(std::make_shared<TestType>(testLoc));
                REQUIRE(tree.Validate());
            }
        }

        SECTION("Multiple items - one oob")
        {
            auto outOfBoundsPoints = { Point{ -1, -1 }, Point{ -1, 1 }, Point{ 1, -1 }, // around top left
                                       Point{ 11, -1 }, Point{ 11, 1 }, Point{ 9, -1 }, // around top right
                                       Point{ 11, 11 }, Point{ 11, 9 }, Point{ 9, 11 }, // around bottom right
                                       Point{ -1, 11 }, Point{ -1, 9 }, Point{ 1, 11 }, // around bottom left
                                       Point{ -100, 11 }, Point{ -1, 100 }, Point{ 100, 110 }, // way out of bounds
                                     };
            for (const auto& testLoc : outOfBoundsPoints) {
                QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);
                for (int i = 0; i < 25; ++i) {
                    tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
                }
                REQUIRE(tree.Validate());

                tree.Insert(std::make_shared<TestType>(testLoc));
                REQUIRE(tree.Validate());
            }
        }
    }

    SECTION("Contract root")
    {
        Rect bounds{ 0, 0, 10, 10 };
        Rect outOfBounds{ 10, 10, 20, 20 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;

        QuadTree<TestType> tree(bounds, targetCount, countLeeway, minQuadSize);

        for (int i = 0; i < 25; ++i) {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(bounds)));
        }

        REQUIRE(tree.Validate());

        for (int i = 0; i < 25; ++i) {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(outOfBounds)));
        }

        REQUIRE(tree.Validate());

        tree.RemoveIf([&](const TestType& item) -> bool
        {
            return Contains(outOfBounds, item.GetLocation());
        });

        REQUIRE(tree.Validate());
    }

    SECTION("Removing items")
    {
        const Rect area{ 0, 0, 10, 10 };
        const Rect topArea{ 0, 0, 10, 5 };
        const Rect bottomArea{ 0, 5, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        const double minQuadSize = 1.0;
        const size_t itemCount = 50;
        QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);

        auto removeTopItemsPredicate = [=](const TestType& item)
        {
            return Contains(topArea, item.GetLocation());
        };

        for (size_t i = 0; i < itemCount / 2; ++i) {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(topArea)));
            tree.Insert(std::make_shared<TestType>(Random::PointIn(bottomArea)));
        }

        SECTION("RemoveIf")
        {
            tree.RemoveIf(removeTopItemsPredicate);

            tree.ForEachItem(ConstQuadTreeIterator<TestType>([bottomArea](const TestType& item)
            {
                REQUIRE(Contains(bottomArea, item.GetLocation()));
            }));

            REQUIRE(tree.Size() == itemCount / 2);
            REQUIRE(tree.Validate());
        }

        SECTION("ForEach predicate")
        {
            tree.ForEachItem(QuadTreeIterator<TestType>([](std::shared_ptr<TestType> /*item*/)
            {
                // Do nothing
            }).SetRemoveItemPredicate(removeTopItemsPredicate));

            tree.ForEachItem(ConstQuadTreeIterator<TestType>([bottomArea](const TestType& item)
            {
                REQUIRE(Contains(bottomArea, item.GetLocation()));
            }));

            REQUIRE(tree.Size() == itemCount / 2);
            REQUIRE(tree.Validate());
        }
    }
    SECTION("ForEach with collide")
    {
        const Rect area{ 0, 0, 10, 10 };
        const Rect topArea{ 0, 0, 10, 5 };
        const Rect bottomArea{ 0, 5, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        const double minQuadSize = 1.0;
        const size_t itemCount = 50;
        const size_t topItemCount = itemCount / 3;
        const size_t bottomItemCount = itemCount - topItemCount;
        QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);

        for (size_t i = 0; i < std::max(topItemCount, bottomItemCount); ++i) {
            if (i < topItemCount) {
                tree.Insert(std::make_shared<TestType>(Random::PointIn(topArea)));
            }
            if (i < bottomItemCount) {
                tree.Insert(std::make_shared<TestType>(Random::PointIn(bottomArea)));
            }
        }

        SECTION("non-const")
        {
            size_t totalCount = 0;
            tree.ForEachItem(QuadTreeIterator<TestType>([&](std::shared_ptr<TestType> /*item*/)
            {
                ++totalCount;
            }).SetItemFilter(area));
            REQUIRE(totalCount == itemCount);

            size_t topCount = 0;
            tree.ForEachItem(QuadTreeIterator<TestType>([&](std::shared_ptr<TestType> item)
            {
                REQUIRE(Contains(topArea, item->GetLocation()));
                ++topCount;
            }).SetItemFilter(topArea));
            REQUIRE(topCount == topItemCount);

            size_t bottomCount = 0;
            tree.ForEachItem(QuadTreeIterator<TestType>([&](std::shared_ptr<TestType> item)
            {
                REQUIRE(Contains(bottomArea, item->GetLocation()));
                ++bottomCount;
            }).SetItemFilter(bottomArea));
            REQUIRE(bottomCount == bottomItemCount);

            REQUIRE(tree.Size() == itemCount);
            REQUIRE(tree.Validate());
        }

        SECTION("const")
        {
            size_t totalCount = 0;
            tree.ForEachItem(ConstQuadTreeIterator<TestType>([&](const TestType& /*item*/)
            {
                ++totalCount;
            }).SetItemFilter(area));
            REQUIRE(totalCount == itemCount);

            size_t topCount = 0;
            tree.ForEachItem(ConstQuadTreeIterator<TestType>([&](const TestType& /*item*/)
            {
                ++topCount;
            }).SetItemFilter(topArea));
            REQUIRE(topCount == topItemCount);

            size_t bottomCount = 0;
            tree.ForEachItem(ConstQuadTreeIterator<TestType>([&](const TestType& /*item*/)
            {
                ++bottomCount;
            }).SetItemFilter(bottomArea));
            REQUIRE(bottomCount == bottomItemCount);

            REQUIRE(tree.Size() == itemCount);
            REQUIRE(tree.Validate());
        }
    }

    SECTION("Moving items")
    {
        const Rect area{ 0, 0, 10, 10 };
        const double minQuadSize = 1.0;
        const size_t itemCount = 50;
        std::vector<std::pair<size_t, size_t>> targetAndLeewayCombinations{
            { 1, 0 },
            { 5, 0 },
            { 5, 5 },
            { 0, itemCount },
            { itemCount, 0 },
            { itemCount, itemCount },
            { 1, 7 }, // make sure it does something sensible!
            { 0, 0 }, // make sure it does something sensible!
            { 0, 7 }, // make sure it does something sensible!
        };
        for (const auto& [ targetCount, countLeeway ] : targetAndLeewayCombinations) {
            QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);

            for (size_t i = 0; i < itemCount; ++i) {
                tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
            }

            auto iter = QuadTreeIterator<TestType>([=](std::shared_ptr<TestType> item)
            {
                item->location_ = Random::PointIn(area);
            });
            tree.ForEachItem(iter);

            REQUIRE(tree.Validate());
            REQUIRE(tree.Size() == itemCount);
        }
    }

    SECTION("Full use-case test")
    {
        const Rect startArea{ 0, 0, 10, 10 };
        const Rect movementArea{ -100, -100, 100, 100 };
        const double minQuadSize = 1.0;
        const size_t itemCount = 100;
        std::vector<std::pair<size_t, size_t>> targetAndLeewayCombinations{
            { 1, 0 },
            { 5, 0 },
            { 5, 5 },
            { 0, itemCount },
            { itemCount, 0 },
            { itemCount, itemCount },
            { 1, 7 }, // make sure it does something sensible!
            { 0, 0 }, // make sure it does something sensible!
            { 0, 7 }, // make sure it does something sensible!
        };
        for (const auto& [ targetCount, countLeeway ] : targetAndLeewayCombinations) {
            QuadTree<TestType> tree(startArea, targetCount, countLeeway, minQuadSize);

            // Now go through a few a few test loops
            for (int i = 0; i < 100; ++i) {
                // Top up the tree, as we may have removed a few items
                size_t itemsToAdd = itemCount - tree.Size();
                for (size_t i = 0; i < itemsToAdd; ++i) {
                    tree.Insert(std::make_shared<TestType>(Random::PointIn(startArea)));
                }

                tree.ForEachItem(QuadTreeIterator<TestType>([=](std::shared_ptr<TestType> item)
                {
                    item->location_ = Random::PointIn(movementArea);
                }).SetRemoveItemPredicate([](const TestType& /*item*/) -> bool
                {
                    return Random::Boolean();
                }));

                REQUIRE(tree.Validate());
                REQUIRE(tree.Size() != itemCount);
            }
        }
    }

    SECTION("Add items mid iteration")
    {
        const Rect area{ 0, 0, 10, 10 };
        const size_t targetCount = 1;
        const size_t countLeeway = 0;
        const double minQuadSize = 1.0;
        const size_t itemCount = 25;
        QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);

        for (size_t i = 0; i < itemCount; ++i) {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
        }

        tree.ForEachItem(QuadTreeIterator<TestType>([&](auto /*item*/)
        {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
        }));

        REQUIRE(tree.Validate());
        REQUIRE(tree.Size() == itemCount * 2);
    }

    SECTION("Const iteration")
    {
        const Rect area{ 0, 0, 10, 10 };
        const size_t targetCount = 1;
        const size_t countLeeway = 0;
        const double minQuadSize = 1.0;
        const size_t itemCount = 25;
        QuadTree<TestType> tree(area, targetCount, countLeeway, minQuadSize);

        for (size_t i = 0; i < itemCount; ++i) {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
        }

        tree.ForEachItem(QuadTreeIterator<TestType>([&](std::shared_ptr<TestType> /*item*/)
        {
            REQUIRE(!tree.Validate());
        }));

        tree.ForEachItem(ConstQuadTreeIterator<TestType>([&](const TestType& /*item*/)
        {
            REQUIRE(tree.Validate());
        }));

        tree.ForEachItem(QuadTreeIterator<TestType>([&](std::shared_ptr<TestType> /*item*/)
        {
            REQUIRE(!tree.Validate());
        }));

        tree.ForEachItem(ConstQuadTreeIterator<TestType>([&](const TestType& /*item*/)
        {
            REQUIRE(tree.Validate());
        }));
    }
}
