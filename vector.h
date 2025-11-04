#pragma once

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <utility>
#include <type_traits>
#include <stdexcept>
#include <iterator>
#include <ranges>

// insert_range & emplace_range aren't implemented, cause my clang doesnt know what is container-compatible-range template & max_size isn't implemented, cause it's
// a bit hard to implement and has almost no use in modern day development

template <typename T>
class Vector{
public:
    // Constructors
    Vector() = default;

    Vector(const Vector& other) {
        size_ = other.size_;
        capacity_ = other.capacity_;
        buffer_.reset();
        std::unique_ptr<T[]> new_buffer = std::make_unique<T[]>(capacity_);
        for (size_t i = 0; i < size_; ++i) {
            new_buffer[i] = other.buffer_[i];
        }
        buffer_ = std::move(new_buffer);
    }

    Vector(Vector&& other) {
        capacity_ = std::move(other.capacity_);
        size_ = std::move(other.size_);
        buffer_ = std::move(other.buffer_);
    }

    Vector(std::initializer_list<T> ilist) {
        for (auto il : ilist) {
            PushBack(il);
        }
    }
    
    ~Vector() {
        Clear();
        buffer_.reset();
    }

    // Assignment & copy
    constexpr Vector& operator=(const Vector& other) {
        if (&other == this) {
            return *this;
        }
        Clear();
        size_ = other.size_;
        capacity_ = other.capacity_;
        buffer_.reset();
        std::unique_ptr<T[]> new_buffer = std::make_unique<T[]>(capacity_);
        for (size_t i = 0; i < size_; ++i) {
            new_buffer[i] = other.buffer_[i];
        }
        buffer_ = std::move(new_buffer);
        return *this;
    }

    constexpr Vector& operator=(Vector&& other) noexcept {
        if (&other == this) {
            return *this;
        }
        size_ = std::move(other.size_);
        capacity_ = std::move(other.capacity_);
        buffer_.reset();
        buffer_ = std::move(other.buffer_);
        return *this;
    }

    constexpr Vector& operator=(std::initializer_list<T> ilist) {
        Clear();
        for (auto il : ilist) {
            PushBack(il);
        }
        return *this;
    }

    // Element access
    T& At(size_t ind) {
        if (ind >= size_) {
            throw(std::out_of_range("Vector index out of range!"));
        }
        return buffer_[ind];
    }

    const T& At(size_t ind) const {
        if (ind >= size_) {
            throw(std::out_of_range("Vector index out of range!"));
        }
        return buffer_[ind];
    }

    // no bounds check on []s, as cppreference says
    T& operator[] (size_t ind) {
        return buffer_[ind];
    }

    const T& operator[] (size_t ind) const {
        return buffer_[ind];
    }

    T& Front() {
        return *buffer_.get(); // as cppreference says.
    };

    const T& Front() const {
        return *buffer_.get();
    }

    T& Back() {
        return buffer_[size_ - 1];
    }
    const T& Back() const {
        return buffer_[size_ - 1];
    }

    T* Data() {
        return buffer_.get();
    }

    const T* Data() const {
        return buffer_.get();
    }

    // Iterators
    T* Begin() {
        return buffer_.get();
    }
    const T* Begin() const {
        return buffer_.get();
    }
    const T* CBegin() const noexcept {
        return buffer_.get();
    }
    
    T* End() {
        return buffer_.get() + size_;
    }
    const T* End() const {
        return buffer_.get() + size_;
    }
    const T* CEnd() const noexcept {
        return buffer_.get() + size_;
    }

    std::reverse_iterator<T*> RBegin() {
        return buffer_.get();
    }
    std::reverse_iterator<const T*> RBegin() const {
        return buffer_.get();
    }
    std::reverse_iterator<const T*> CRBegin() const noexcept {
        return buffer_.get();
    }
    
    std::reverse_iterator<T*> REnd() {
        return buffer_.get() + size_;
    }
    std::reverse_iterator<const T*> REnd() const {
        return buffer_.get() + size_;
    }
    std::reverse_iterator<const T*> CREnd() const noexcept {
        return buffer_.get() + size_;
    }
    
    // Capacity
    bool Empty() const {
        return size_ == 0;
    }
    size_t Size() const {
        return size_;
    }
    void Reserve(size_t new_cap) {
        if (new_cap > capacity_) reallocate(new_cap);
    }
    size_t Capacity() const {
        return capacity_;
    }
    void ShrinkToFit() {
        if (size_) reallocate(size_);
    }

    // Modifiers
    bool Clear() {
        size_ = 0;
        capacity_ = 0;
        buffer_.reset();
        buffer_ = nullptr;
    }

    T* Insert(const T* pos, const T& value) {
        if (size_ == capacity_) {
            reallocate();
        }
        size_t ind = static_cast<size_t>(std::distance(buffer_.get(), pos));
        if (ind > size_) {
            return pos;
        }

        for (size_t i = size_; i > ind; --i) {
            buffer_[i] = buffer_[i-1];
        }
        buffer_[ind] = value;
        ++size_;
        return buffer_[ind];
    }

    T* Insert(const T* pos, T&& value) {
        if (size_ == capacity_) {
            reallocate();
        }
        size_t ind = static_cast<size_t>(std::distance(buffer_.get(), pos));
        if (ind > size_) {
            return pos;
        }

        for (size_t i = size_; i > ind; --i) {
            buffer_[i] = buffer_[i-1];
        }
        buffer_[ind] = std::move(value);
        ++size_;
        return buffer_[ind];
    }

    T* Insert(const T* pos, size_t count, const T& value) {
        while (size_ + count > capacity_) {
            reallocate();
        }
        size_t ind = static_cast<size_t>(std::distance(buffer_.get(), pos));
        if (ind > size_) {
            return pos;
        }

        for (size_t i = size_ + count - 1; i > ind + count - 1; --i) {
            buffer_[i] = buffer_[i-count];
        }
        for (size_t i = ind; i < ind + count; ++i) {
             buffer_[i] = value;
        }
        size_ += count;
        return buffer_[ind];
    }

    T* Insert(const T* pos, std::initializer_list<T> ilist) {
        size_t ind = static_cast<size_t>(std::distance(buffer_.get(), pos));
        size_t count = ilist.size();
        if (ind > size_) {
            return pos;
        }

        while (size_ + count > capacity_) {
            reallocate();
        }
        for (size_t i = size_ + count - 1; i > ind + count - 1; --i) {
            buffer_[i] = buffer_[i-count];
        }
        size_t i = ind;
        for (auto it : ilist) {
            buffer_[i] = *it;
            i++;
        }
        size_ += count;
        return buffer_[ind];
    }

    template <std::input_iterator InputIt>
    T* Insert(const T* pos, InputIt first, InputIt last) {
        size_t ind = static_cast<size_t>(std::distance(buffer_.get(), pos));
        if (ind > size_) {
            return pos;
        }
        size_t count = static_cast<size_t>(std::distance(first, last));
        while (size_ + count > capacity_) {
            reallocate();
        }
        for (size_t i = size_ + count - 1; i > ind + count - 1; --i) {
            buffer_[i] = buffer_[i-count];
        }
        size_t i = ind;
        for (auto it = first; it != last; it++) {
            buffer_[i] = *it;
            i++;
        }
        size_ += count;
        return buffer_[ind];
    }

    template <class... Args>
    T* Emplace(const T* pos, Args&&... args) {
        if (size_ == capacity_) {
            reallocate();
        }
        size_t ind = static_cast<size_t>(std::distance(buffer_.get(), pos));
        if (ind > size_) {
            return pos;
        }

        for (size_t i = size_; i > ind; --i) {
            buffer_[i] = buffer_[i-1];
        }
        ++size_;
        std::allocator_traits<std::allocator<T>>::construct(std::allocator<T>::allocator, buffer_.get() + ind, std::forward<Args>(args)...);
        return buffer_[ind];
    }

    T* Erase(const T* pos) {
        size_t ind = static_cast<size_t>(std::distance(buffer_.get(), pos));
        if (ind == size_ - 1) {
            return End();
        }

        for (size_t i = ind; i < size_ - 1; ++i) {
            buffer_[i] = buffer_[i+1];
        }
        --size_;
        return ind;
    };
 
    T* Erase(const T* first, const T* last) {
        size_t ind2 = static_cast<size_t>(std::distance(buffer_.get(), last));
        if (ind2 == size_ - 1) {
            return End;
        }
        if (std::distance(first, last) == 0) {
            return last;
        }
        size_t ind1 = static_cast<size_t>(std::distance(buffer_.get(), first));
        for (size_t i = ind1; i < size_ - std::distance(first, last); ++i) {
            buffer_[i] = buffer_[i + std::distance(first, last)];
        }
        return buffer_[size_ - std::distance(first, last) - 1];
        size_ -= std::distance(first, last);
    }

    void PushBack(const T& value) {
        if (size_ == capacity_) {
            reallocate();
        }
        buffer_[size_] = value;
        ++size_;
    }
    void PushBack(T&& value) {
        if (size_ == capacity_) {
            reallocate();
        }
        buffer_[size_] = std::move(value);
        ++size_;
    }

    template <class... Args>
    constexpr T& EmplaceBack(Args&&... args) {
        if (size_ == capacity_) {
            reallocate();
        }
        std::allocator_traits<std::allocator<T>>::construct(std::allocator<T>::allocator, buffer_.get() + size_, std::forward<Args>(args)...);
        ++size_;
        return buffer_[size_ - 1];
    }

    void PopBack() {
        --size_;
    }

    void Resize(size_t count) {
        while (count > capacity_) {
            reallocate();
        }
        if (count > size_) {
            for (size_t i = size_; i < count; ++i) {
                buffer_[i] = T();
            }
        }
        size_ = count;
    }
    void Resize(size_t count, T& value) {
        while (count > capacity_) {
            reallocate();
        }
        if (count > size_) {
            for (size_t i = size_; i < count; ++i) {
                buffer_[i] = value;
            }
        }
        size_ = count;
    }
    
    void Swap(Vector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(buffer_, other.buffer_);
    }

private:
    std::unique_ptr<T[]> buffer_[0] = nullptr;
    size_t capacity_ = 0;
    size_t size_ = 0;

    void reallocate(size_t new_cap = 0) {
        new_cap ? capacity_ = new_cap : capacity_ = std::max<size_t>(capacity_ * 2, 1);
        std::unique_ptr<T[]> new_buffer_ = std::make_unique<T[]>(capacity_);
        for (size_t i = 0; i < size_; ++i) {
            new_buffer_[i] = std::move(buffer_[i]);
        }
        buffer_ = std::move(new_buffer_);
    }
};
