#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <vector>
#include <functional>

namespace util {

template <typename T>
class CircularBuffer {
public:
    struct Iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;

        Iterator(std::vector<T>& items, size_t startIndex, size_t iterationCount)
            : underlyingContainer_(items)
            , iterationCount_(0)
            , iterationWrapIndex_(items.size() - startIndex)
            , iterationMax_(iterationCount)
        {
            if (iterationCount == 0) {
                iter_ = std::end(underlyingContainer_);
            } else {
                iter_ = std::begin(underlyingContainer_);
                std::advance(iter_, startIndex);
            }
        }

        reference operator*() const { return *iter_; }
        pointer operator->() { return iter_; }
        Iterator& operator++()
        {
            ++iter_;
            ++iterationCount_;
            if (iterationCount_ >= iterationMax_) {
                iter_ = std::end(underlyingContainer_);
            } else if (iterationCount_ == iterationWrapIndex_) {
                iter_ = std::begin(underlyingContainer_);
            }
            return *this;
        }
        Iterator operator++(int) { Iterator copy = *this; ++(*this); return copy; }
        friend bool operator== (const Iterator& a, const Iterator& b) { return a.iter_ == b.iter_; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.iter_ != b.iter_; };

    private:
        std::vector<T>& underlyingContainer_;
        typename std::vector<T>::iterator iter_;
        size_t iterationCount_; // counts up sequentially
        const size_t iterationWrapIndex_; // count where dataPointer jumps from end of container to start
        const size_t iterationMax_; // indication that we should equal end()
    };

    Iterator begin()
    {
        bool full = fill_ == items_.size();
        size_t startIndex = full ? next_ : 0;
        return Iterator(items_, startIndex, fill_);
    }
    Iterator end()
    {
        return Iterator(items_, items_.size(), 0);
    }

    CircularBuffer(size_t capacity)
        : items_(capacity, T{})
        , next_(0)
        , fill_(0)
    {
        items_.shrink_to_fit();
    }

    size_t Size() const { return fill_; }
    size_t Capacity() const { return items_.size(); }
    bool Full() const { return Size() == Capacity(); }
    bool Empty() const { return Size() == 0; }

    T Oldest() const
    {
        if (!items_.empty()) {
            if (Full()) {
                return items_[next_];
            } else {
                return items_.front();
            }
        } else {
            return {};
        }
    }
    T Newest() const
    {
        if (!items_.empty()) {
            if (next_ > 0) {
                return items_[next_ - 1];
            } else {
                return items_.back();
            }
        } else {
            return {};
        }
    }

    void Resize(size_t size)
    {
        CircularBuffer copy(items_.size());
        copy.items_.swap(items_);
        copy.next_ = next_;
        copy.fill_ = fill_;

        items_.resize(size);
        Clear();

        for (const T& item : copy) {
            PushBack(item);
        }
    }
    void PushBack(const T& item)
    {
        if (items_.size() > 0) {
            items_[next_++] = item;
            if (next_ == items_.size()) {
                next_ = 0;
            }
            if (fill_ < items_.size()) {
                ++fill_;
            }
        }
    }
    void Clear()
    {
        std::fill(std::begin(items_), std::end(items_), T{});
        next_ = 0;
        fill_ = 0;
    }

private:
    std::vector<T> items_;
    size_t next_;
    size_t fill_;
};

} // end namespace util

#endif // CIRCULARBUFFER_H
