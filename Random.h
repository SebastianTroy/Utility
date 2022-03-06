#ifndef RANDOM_H
#define RANDOM_H

#include "Shape.h"
#include "Algorithm.h"

#include <random>
#include <limits>
#include <vector>
#include <algorithm>
#include <functional>
#include <numbers>

class Random {
public:
    template<typename T>
    class WeightedContainer {
    public:
        void PushBack(T&& value, const double& weight)
        {
            values_.push_back(std::move(value));
            weights_.push_back(weight);
            distribution_ = std::discrete_distribution<size_t>(std::begin(weights_), std::end(weights_));
        }

        const T& RandomItem()
        {
            auto index = Random::Generate(distribution_);
            return values_.at(index);
        }

    private:
        std::vector<T> values_;
        std::vector<double> weights_;
        std::discrete_distribution<size_t> distribution_;
    };

    static void Seed(const std::mt19937::result_type& seed)
    {
        entropy_.seed(seed);
    }

    static double Bearing()
    {
        return Random::Number(0.0, std::numbers::pi * 2.0);
    }

    static bool Boolean()
    {
        std::bernoulli_distribution d;
        return Generate(d);
    }

    template<typename NumericType>
    static NumericType Sign(const NumericType& value)
    {
        return Boolean() ? value : -value;
    }

    static Point PointIn(const Line& line)
    {
        double proportion = Random::Proportion();
        double deltaX = (line.b.x - line.a.x) * proportion;
        double deltaY = (line.b.y - line.a.y) * proportion;
        return { line.a.x + deltaX, line.a.y + deltaY };
    }

    static Point PointIn(const Rect& rect)
    {
        return { Random::Number(util::Range(rect.left, rect.right)), Random::Number(util::Range(rect.bottom, rect.top)) };
    }

    static Point PointIn(const Circle& circle)
    {
        // TODO check to see if while (!RandomPointInSquare.isInCircle()) performs better
        double rotation = Random::Number(0.0, std::numbers::pi * 2.0);
        double distance = std::max(Random::Number(0.0, circle.radius), Random::Number(0.0, circle.radius));
        double x = circle.x + distance * std::cos(rotation);
        double y = circle.y + distance * std::sin(rotation);
        return { x, y };
    }

    template<typename NumericType>
    requires std::is_integral_v<NumericType>
    static NumericType Number(NumericType min, NumericType max)
    {
        std::uniform_int_distribution<NumericType> distribution(min, max);
        return Generate(distribution);
    }

    template<typename NumericType>
    requires std::is_floating_point_v<NumericType>
    static NumericType Number(NumericType min, NumericType max)
    {
        std::uniform_real_distribution<NumericType> distribution(min, std::nextafter(max, std::numeric_limits<NumericType>::max()));
        return Generate(distribution);
    }

    template<typename NumericType>
    static NumericType Number(const util::Range<NumericType>& range)
    {
        return Number<NumericType>(range.Min(), range.Max());
    }

    static size_t WeightedIndex(std::initializer_list<double>&& weights)
    {
        std::discrete_distribution<size_t> d(std::move(weights));
        return Generate(d);
    }

    static double Proportion()
    {
        return Number(0.0, 1.0);
    }

    static double Percent()
    {
        return Number(0.0, 100.0);
    }

    static bool PercentChance(double chance)
    {
        std::bernoulli_distribution d(std::clamp(chance / 100.0, 0.0, 1.0));
        return Generate(d);
    }

    /**
     * Instead of rounding n.0 to n.49... down and n.5 to n.9... up, this
     * function uses the value after the decimal as the propotional chance the
     * value will be rounded up. e.g. 0.1 has a 10% chance of being rounded up
     * to 1, whereas 0.9 has a 90% chance of being rounded up.
     */
    static uint64_t Round(const double& v)
    {
        return static_cast<uint64_t>(v) + (Random::PercentChance(std::fmod(v, 1.0) * 100) ? 1 : 0);
    }

    template<typename NumericType>
    static NumericType Gaussian(NumericType mean = std::numeric_limits<NumericType>::min(), NumericType standardDeviation = NumericType{ 1.0 })
    {
        std::normal_distribution<NumericType> distribution(mean, std::abs(standardDeviation));
        return Generate(distribution);
    }

    template<typename NumericType>
    static NumericType Poisson(NumericType mean = std::numeric_limits<NumericType>::min())
    {
        std::poisson_distribution<NumericType> distribution(mean);
        return Generate(distribution);
    }

    template<typename NumericType>
    requires std::is_integral_v<NumericType>
    static std::vector<NumericType> Numbers(typename std::vector<NumericType>::size_type count, NumericType min, NumericType max)
    {
        std::vector<NumericType> rands;
        rands.reserve(count);

        std::uniform_int_distribution<NumericType> distribution(min, max);
        std::generate_n(std::back_inserter(rands), count, [&](){ return Generate(distribution); });

        return rands;
    }

    template<typename NumericType>
    requires std::is_floating_point_v<NumericType>
    static std::vector<NumericType> Numbers(typename std::vector<NumericType>::size_type count, NumericType min, NumericType max)
    {
        std::vector<NumericType> rands;
        rands.reserve(count);

        std::uniform_real_distribution<NumericType> distribution(min, max);
        std::generate_n(std::back_inserter(rands), count, [&](){ return Generate(distribution); });

        return rands;
    }

    template<typename NumericType>
    static std::vector<NumericType> Gaussians(typename std::vector<NumericType>::size_type count, NumericType mean = std::numeric_limits<NumericType>::min(), NumericType standardDeviation = NumericType{ 1.0 })
    {
        std::normal_distribution<NumericType> distribution(mean, standardDeviation);
        std::vector<NumericType> rands;
        rands.reserve(count);
        std::generate_n(std::back_inserter(rands), count, [&](){ return Generate(distribution); });
        return rands;
    }

    template<typename NumericType>
    static std::vector<NumericType> DualPeakGaussians(typename std::vector<NumericType>::size_type count, NumericType meanPeakOne, NumericType standardDeviationPeakOne, NumericType meanPeakTwo, NumericType standardDeviationPeakTwo)
    {
        std::normal_distribution<NumericType> distributionOne(meanPeakOne, std::abs(standardDeviationPeakOne));
        std::normal_distribution<NumericType> distributionTwo(meanPeakTwo, std::abs(standardDeviationPeakTwo));
        std::vector<NumericType> rands;
        rands.reserve(count);
        std::generate_n(std::back_inserter(rands), count, [&](){ return Random::Boolean() ? Generate(distributionOne) : Generate(distributionTwo); });
        return rands;
    }

    template<typename NumericType>
    static std::vector<NumericType> Poissons(typename std::vector<NumericType>::size_type count, NumericType mean = std::numeric_limits<NumericType>::min())
    {
        std::poisson_distribution<NumericType> distribution(mean);
        std::vector<NumericType> rands;
        rands.reserve(count);
        std::generate_n(std::back_inserter(rands), count, [&](){ return Generate(distribution); });
        return rands;
    }

    template<typename NumericType>
    static NumericType GaussianAdjustment(const NumericType& toAdjust, double proportion)
    {
        return toAdjust + Gaussian(0.0, std::max(0.001, std::abs(toAdjust) * (proportion / 3.0)));
    }

    template<typename Container>
    static void Shuffle(Container& toShuffle)
    {
        std::shuffle(std::begin(toShuffle), std::end(toShuffle), entropy_);
    }

    /**
     * Returns a Container with max(a.size, b.size) items. For each index it
     * will contain a copy of either a[index] or b[index]. Once the max index of
     * the shortest container has been reached, all remaining items are copied
     * from the longer container.
     */
    template<typename Container>
    static Container Merge(const Container& a, const Container& b)
    {
        Container c;
        c.reserve(std::max(a.size(), b.size()));

        util::IterateBoth(a, b, [&c](const auto& a, const auto& b)
        {
            c.push_back(Boolean() ? a : b);
        });

        if (a.size() > b.size()) {
            auto remainingItemsIter = a.cbegin();
            std::advance(remainingItemsIter, a.size() - b.size());
            std::copy(remainingItemsIter, std::cend(a), std::back_inserter(c));
        } else if (b.size() > a.size()) {
            auto remainingItemsIter = b.cbegin();
            std::advance(remainingItemsIter, b.size() - a.size());
            std::copy(remainingItemsIter, std::cend(b), std::back_inserter(c));
        }

        return c;
    }

    template<typename Container>
    static typename Container::value_type& Item(Container& container)
    {
        assert(!container.empty());
        auto iter = std::begin(container);
        std::advance(iter, Random::Number(typename Container::size_type{ 0 }, container.size() - 1));
        return *iter;
    }

    template<typename Container>
    static const typename Container::value_type& Item(const Container& container)
    {
        assert(!container.empty());
        auto iter = std::cbegin(container);
        // THIS SHOULD WORK WITH `unsigned long long` BUT IT DOESN'T (gives undefined behaviour and agressive code deletion in release mode!)
        // FIXME add in a compiler version specific fix for this, seems to be an old GCC bug
        std::advance(iter, Random::Number<unsigned long>(0, container.size() - 1));
        return *iter;
    }

    template<typename Container>
    static void ForNItems(Container& container, size_t itemCount, const std::function<void(typename Container::value_type& item)>& action)
    {
        assert(!container.empty());
        for (size_t count = 0; count < itemCount; ++count) {
            std::invoke(action, Item(container));
        }
    }

    template<typename Container>
    static void ForNItems(const Container& container, size_t itemCount, const std::function<void(const typename Container::value_type& item)>& action)
    {
        assert(!container.empty());
        for (size_t count = 0; count < itemCount; ++count) {
            std::invoke(action, Item(container));
        }
    }

private:
    inline static std::mt19937 entropy_ = std::mt19937();

    template<typename DistributionType>
    static typename DistributionType::result_type Generate(DistributionType& distribution)
    {
        return distribution(entropy_);
    }
};

#endif // RANDOM_H
