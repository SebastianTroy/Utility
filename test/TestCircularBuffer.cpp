#include <CircularBuffer.h>
#include <Random.h>
#include <Algorithm.h>

#include <catch2/catch.hpp>

#include <algorithm>

using namespace util;

TEST_CASE("CircularBuffer", "[container]")
{
    Random::Seed(42);

    SECTION("Constructor")
    {
        std::initializer_list<size_t> capacitiesToTest = { 0, 1, 5, 10, 100, 250 };
        for (size_t capacity : capacitiesToTest) {
            CircularBuffer<int> testBuffer(capacity);
            REQUIRE(testBuffer.Capacity() == capacity);
            REQUIRE(testBuffer.Size() == 0);
            REQUIRE(testBuffer.Empty() == true);
            REQUIRE(testBuffer.Full() == (capacity == 0));
            REQUIRE(testBuffer.Newest() == int{});
            REQUIRE(testBuffer.Oldest() == int{});
        }
    }

    SECTION("Zero sized buffer")
    {
        CircularBuffer<int> testBuffer(0);

        REQUIRE(testBuffer.Capacity() == 0);
        REQUIRE(testBuffer.Size() == 0);
        REQUIRE(testBuffer.Empty() == true);
        REQUIRE(testBuffer.Full() == true);
        REQUIRE(testBuffer.Newest() == int{});
        REQUIRE(testBuffer.Oldest() == int{});

        testBuffer.PushBack(3);

        REQUIRE(testBuffer.Capacity() == 0);
        REQUIRE(testBuffer.Size() == 0);
        REQUIRE(testBuffer.Empty() == true);
        REQUIRE(testBuffer.Full() == true);
        REQUIRE(testBuffer.Newest() == int{});
        REQUIRE(testBuffer.Oldest() == int{});

        testBuffer.Clear();

        REQUIRE(testBuffer.Capacity() == 0);
        REQUIRE(testBuffer.Size() == 0);
        REQUIRE(testBuffer.Empty() == true);
        REQUIRE(testBuffer.Full() == true);
        REQUIRE(testBuffer.Newest() == int{});
        REQUIRE(testBuffer.Oldest() == int{});

        unsigned count = 0;
        for (auto i : testBuffer) {
            ++count;
        }

        REQUIRE(count == 0);
        REQUIRE(testBuffer.Capacity() == 0);
        REQUIRE(testBuffer.Size() == 0);
        REQUIRE(testBuffer.Empty() == true);
        REQUIRE(testBuffer.Full() == true);
        REQUIRE(testBuffer.Newest() == int{});
        REQUIRE(testBuffer.Oldest() == int{});

        testBuffer.Clear();

        REQUIRE(count == 0);
        REQUIRE(testBuffer.Capacity() == 0);
        REQUIRE(testBuffer.Size() == 0);
        REQUIRE(testBuffer.Empty() == true);
        REQUIRE(testBuffer.Full() == true);
        REQUIRE(testBuffer.Newest() == int{});
        REQUIRE(testBuffer.Oldest() == int{});
    }

    SECTION("Filling to capacity")
    {
        std::initializer_list<int> valuesToInsert = { -3, -2, -1, 0, 1, 2, 3, 4, 5, 654, -5431, std::numeric_limits<int>::max(), std::numeric_limits<int>::min(), std::numeric_limits<int>::lowest() };
        CircularBuffer<int> testBuffer(valuesToInsert.size());

        size_t count = 0;
        for (int value : valuesToInsert) {
            testBuffer.PushBack(value);
            REQUIRE(testBuffer.Size() == ++count);
            REQUIRE(testBuffer.Capacity() == valuesToInsert.size());
            REQUIRE(testBuffer.Empty() == false);
            REQUIRE(testBuffer.Full() == (count == valuesToInsert.size()));
            REQUIRE(testBuffer.Newest() == value);
            REQUIRE(testBuffer.Oldest() == *valuesToInsert.begin());
        }
    }

    SECTION("Overwriting")
    {
        constexpr size_t size = 11;
        CircularBuffer<int> testBuffer(size);


        std::initializer_list<int> valuesToFill = { -3, -2, -1, 0, 1, 2, 3, 4, 5, 654, -5431, std::numeric_limits<int>::max(), std::numeric_limits<int>::min(), std::numeric_limits<int>::lowest() };

        for (int valueToTest : valuesToFill) {
            for (unsigned count = 0; count < size; ++count) {
                testBuffer.PushBack(valueToTest);
            }

            REQUIRE(testBuffer.Oldest() == valueToTest);
            REQUIRE(testBuffer.Newest() == valueToTest);

            for (int value : testBuffer) {
                REQUIRE(value == valueToTest);
            };
        }

    }

    SECTION("Clear")
    {
        constexpr size_t testCapacity = 44;

        CircularBuffer<int> testBuffer(testCapacity);

        REQUIRE(testBuffer.Capacity() == testCapacity);
        REQUIRE(testBuffer.Size() == 0);
        testBuffer.Clear();
        REQUIRE(testBuffer.Capacity() == testCapacity);
        REQUIRE(testBuffer.Size() == 0);
        REQUIRE(testBuffer.Oldest() == int{});
        REQUIRE(testBuffer.Newest() == int{});


        testBuffer.PushBack(Random::Number<int>(-1000, 1000));
        REQUIRE(testBuffer.Capacity() == testCapacity);
        REQUIRE(testBuffer.Size() == 1);
        testBuffer.Clear();
        REQUIRE(testBuffer.Capacity() == testCapacity);
        REQUIRE(testBuffer.Size() == 0);
        REQUIRE(testBuffer.Oldest() == int{});
        REQUIRE(testBuffer.Newest() == int{});

        testBuffer.PushBack(Random::Number<int>(-1000, 1000));
        testBuffer.PushBack(Random::Number<int>(-1000, 1000));
        testBuffer.PushBack(Random::Number<int>(-1000, 1000));
        testBuffer.PushBack(Random::Number<int>(-1000, 1000));
        REQUIRE(testBuffer.Capacity() == testCapacity);
        REQUIRE(testBuffer.Size() == 4);
        testBuffer.Clear();
        REQUIRE(testBuffer.Capacity() == testCapacity);
        REQUIRE(testBuffer.Size() == 0);
        REQUIRE(testBuffer.Oldest() == int{});
        REQUIRE(testBuffer.Newest() == int{});

        for (size_t i = 0; i < testCapacity * 2; ++i) {
            testBuffer.PushBack(Random::Number<int>(-1000, 1000));
        }
        REQUIRE(testBuffer.Capacity() == testCapacity);
        REQUIRE(testBuffer.Size() == testCapacity);
        testBuffer.Clear();
        REQUIRE(testBuffer.Capacity() == testCapacity);
        REQUIRE(testBuffer.Size() == 0);
        REQUIRE(testBuffer.Oldest() == int{});
        REQUIRE(testBuffer.Newest() == int{});
    }

    SECTION("ForEach")
    {
        for (size_t testSize : { 0, 1, 2, 4, 8, 16, 32, 64, 7, 9, 11, 201 }) {
            CircularBuffer<int> testBuffer(testSize);

            size_t countPushedBack = 0;
            do {
                size_t count = 0;
                for (int i : testBuffer) {
                    ++count;
                }
                REQUIRE(count == countPushedBack);
                REQUIRE(count == testBuffer.Size());

                testBuffer.PushBack({});
                ++countPushedBack;
            } while (!testBuffer.Full());
        }
    }

    SECTION("Oldest / Newest")
    {
        constexpr size_t testSize = 15;
        const std::vector<int> testValues = Random::Numbers<int>(testSize * 3, -100000, 100000);

        CircularBuffer<int> testBuffer(testSize);

        REQUIRE(testBuffer.Oldest() == int{});
        REQUIRE(testBuffer.Newest() == int{});

        size_t index = 0;
        for (int testValue : testValues) {
            testBuffer.PushBack(testValue);
            size_t expectedOldest;
            if (index < testSize) {
                expectedOldest = 0;
            } else {
                expectedOldest = 1 + index - testSize;
            }
            REQUIRE(testBuffer.Oldest() == testValues.at(expectedOldest));
            REQUIRE(testBuffer.Newest() == testValue);
            ++index;
        }
    }

    SECTION("Set Capacity - Capacity/Oldest/Newest/Full/Empty")
    {
        for (size_t capacity : { 0, 1, 5, 10, 100, 250, 100, 10, 5, 1, 0, 250, 0 }) {
            CircularBuffer<int> testBuffer(0);
            testBuffer.Resize(capacity);
            REQUIRE(testBuffer.Capacity() == capacity);
        }

        constexpr size_t size = 40;
        auto values = Random::Numbers<int>(size, -10000, 10000);
        CircularBuffer<int> testBuffer(size);
        REQUIRE(testBuffer.Empty());
        REQUIRE(!testBuffer.Full());
        for (auto value : values) {
            testBuffer.PushBack(value);
        }

        REQUIRE(testBuffer.Oldest() == values.front());
        REQUIRE(testBuffer.Newest() == values.back());
        REQUIRE(!testBuffer.Empty());
        REQUIRE(testBuffer.Full());

        testBuffer.Resize(size + 1);
        REQUIRE(testBuffer.Capacity() == size + 1);

        REQUIRE(testBuffer.Oldest() == values.front());
        REQUIRE(testBuffer.Newest() == values.back());
        REQUIRE(!testBuffer.Empty());
        REQUIRE(!testBuffer.Full());

        testBuffer.Resize(size * 2);
        REQUIRE(testBuffer.Capacity() == size * 2);

        REQUIRE(testBuffer.Oldest() == values.front());
        REQUIRE(testBuffer.Newest() == values.back());
        REQUIRE(!testBuffer.Empty());
        REQUIRE(!testBuffer.Full());

        testBuffer.Resize(size);
        REQUIRE(testBuffer.Capacity() == size);

        REQUIRE(testBuffer.Oldest() == values.front());
        REQUIRE(testBuffer.Newest() == values.back());
        REQUIRE(!testBuffer.Empty());
        REQUIRE(testBuffer.Full());

        testBuffer.Resize(size / 2);
        REQUIRE(testBuffer.Capacity() == size / 2);

        REQUIRE(testBuffer.Oldest() == values.at((size / 2)));
        REQUIRE(testBuffer.Newest() == values.back());
        REQUIRE(!testBuffer.Empty());
        REQUIRE(testBuffer.Full());

        testBuffer.Resize(size);
        REQUIRE(testBuffer.Capacity() == size);

        REQUIRE(testBuffer.Oldest() == values.at((size / 2)));
        REQUIRE(testBuffer.Newest() == values.back());
        REQUIRE(!testBuffer.Empty());
        REQUIRE(!testBuffer.Full());

        testBuffer.Resize(size / 2);
        REQUIRE(testBuffer.Capacity() == size / 2);

        REQUIRE(testBuffer.Oldest() == values.at((size / 2)));
        REQUIRE(testBuffer.Newest() == values.back());
        REQUIRE(!testBuffer.Empty());
        REQUIRE(testBuffer.Full());
    }

    SECTION("PushBack")
    {
        for (auto testSize : { 0, 1, 2, 11, 32, 64, 113 }) {
            auto testValues = Random::Numbers<int>(1 + (testSize * 3), -10000, 10000);
            CircularBuffer<int> testBuffer(testSize);

            std::vector<int> pushedBack;
            for (auto testValue : testValues) {
                pushedBack.push_back(testValue);
                testBuffer.PushBack(testValue);
                // make sure that when circular buffer begins erasing values, we erase values from the test buffer
                while (pushedBack.size() > testBuffer.Size()) {
                    pushedBack.erase(std::begin(pushedBack));
                }

                std::vector<int> bufferCopy;
                for (auto i : testBuffer) {
                    bufferCopy.push_back(i);
                }
                REQUIRE(bufferCopy == pushedBack);
            }
        }
    }

    SECTION("Set Capacity - Content order maintained")
    {
        constexpr size_t size = 40;
        auto testValues = Random::Numbers<int>(size, -10000, 10000);
        CircularBuffer<int> testBuffer(size);
        for (size_t startIndex = 0; startIndex <= size; ++startIndex) {
            // Add some random crap we are about to overwrite (Just want the test values in the buffer, but not aligned to the start)
            // We want to test that no matter where our start index is within the circular buffer, it all works as planned
            testBuffer.Clear();
            for (size_t i = 0; i < startIndex; ++i) {
                testBuffer.PushBack(42);
            }

            for (auto value : testValues) {
                testBuffer.PushBack(value);
            }

            do {
                std::vector<int> copy;
                for (int value : testBuffer) {
                    copy.push_back(value);
                }
                REQUIRE(copy == testValues);
                if (!testValues.empty()) {
                    testValues.erase(std::begin(testValues));
                    testBuffer.Resize(testValues.size());
                    REQUIRE(testBuffer.Capacity() == testValues.size());
                }
            } while (!testValues.empty());
        }
    }
}
