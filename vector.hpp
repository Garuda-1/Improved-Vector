#ifndef EXAM_VECTOR_VECTOR_HPP
#define EXAM_VECTOR_VECTOR_HPP

#include <variant>
#include <cstdint>
#include "asp.hpp"
#include "vector_iterator.hpp"

template<typename T>
struct vector {
    typedef T value_type;

    typedef vector_iterator<T> iterator;
    typedef vector_const_iterator<T> const_iterator;
    typedef std::reverse_iterator<vector_iterator<T>> reverse_iterator;
    typedef std::reverse_iterator<vector_const_iterator<T>> const_reverse_iterator;

    std::variant<asp<T>, T> src_;

    vector() = default;

    vector(vector const& that) = default;

    // TODO: InputIterator ctor

    T& operator[](size_t i) {
        if (get_status() == SMALL) {
            return std::get<1>(src_);
        } else {
            return std::get<0>(src_)[i];
        }
    }

    const T operator[](size_t i) const {
        if (get_status() == SMALL) {
            return std::get<1>(src_);
        } else {
            return std::get<0>(src_)[i];
        }
    }

    T& front() {
        if (get_status() == SMALL) {
            return std::get<1>(src_);
        } else {
            return std::get<0>(src_)[0];
        }
    }

    const T& front() const {
        if (get_status() == SMALL) {
            return std::get<1>(src_);
        } else {
            return std::get<0>(src_)[0];
        }
    }

    T& back() {
        if (get_status() == SMALL) {
            return std::get<1>(src_);
        } else {
            size_t size = std::get<0>(src_).get_size();
            return std::get<0>(src_)[size - 1];
        }
    }

    const T& back() const {
        if (get_status() == SMALL) {
            return std::get<1>(src_);
        } else {
            size_t size = std::get<0>(src_).get_size();
            return std::get<0>(src_)[size - 1];
        }
    }

    void push_back(const T& value) {
        switch (get_status()) {
            case EMPTY: {
                src_.template emplace<1>(value);
                return;
            }
            case SMALL: {
                allocate();     // Exception alert
                size_t& size = std::get<0>(src_).get_size();
                new (&std::get<0>(src_)[size]) T(value);    // Exception alert
                ++size;
                return;
            }
            case ALLOCATED: {
                size_t size = std::get<0>(src_).get_size();
                size_t cap = std::get<0>(src_).get_cap();
                if (size == cap) {
                    reallocate(cap * 2);
                }
                new (&std::get<0>(src_)[size]) T(value);
                ++std::get<0>(src_).get_size();
                return;
            }
        }
    }

    void pop_back() {
        switch (get_status()) {
            case EMPTY: {
                return;
            }
            case SMALL: {
                src_.template emplace<0>();
                return;
            }
            case ALLOCATED: {
                size_t& size = std::get<0>(src_).get_size();
                std::get<0>(src_)[size - 1].~T();
                --size;
            }
        }
    }

    T* data() {
        if (get_status() == SMALL) {
            return &(std::get<1>(src_));
        } else {
            return std::get<0>(src_).get_data();
        }
    }

    const T* data() const {
        if (get_status() == SMALL) {
            return &(std::get<1>(src_));
        } else {
            return std::get<0>(src_).get_data();
        }
    }

    bool empty() const {
        return size() == 0;
    }

    size_t size() const {
        switch (get_status()) {
            case EMPTY: {
                return 0;
            }
            case SMALL: {
                return 1;
            }
            case ALLOCATED: {
                return std::get<0>(src_).get_size();
            }
        }
    }

    void reserve(size_t cap) {
        if (cap > 1) {
            if (get_status() == ALLOCATED) {
                reallocate(cap);
            } else {
                try {
                    allocate(cap);
                } catch (std::exception &e) {
                    throw e;
                }
            }
        }
    }

    size_t capacity() const {
        if (get_status() == ALLOCATED) {
            return std::get<0>(src_).get_cap();
        } else {
            return 1;
        }
    }

    void shrink_to_fit() {
        if (get_status() == ALLOCATED) {
            size_t size = std::get<0>(src_).get_size();
            reallocate(size);
        }
    }

    void resize(size_t new_size, const T& val = T()) {
        if (size() < new_size) {
            if (new_size == 1 && get_status() == EMPTY) {
                src_.template emplace<T>(val);
            } else {
                size_t cap = capacity();
                if (cap < new_size) {
                    while (cap < new_size) {
                        cap *= 2;
                    }
                    if (get_status() == ALLOCATED) {
                        reallocate(cap);
                    } else {
                        try {
                            allocate(cap);
                        } catch (std::exception &e) {
                            throw e;
                        }
                    }
                }
                while (size() < new_size) {
                    push_back(val);
                }
            }
        } else {
            switch (get_status()) {
                case ALLOCATED: {
                    for (size_t i = new_size; i < size(); ++i) {
                        std::get<0>(src_)[i].~T();
                    }
                    std::get<0>(src_).get_size() = new_size;
                    return;
                }
                case SMALL: {
                    src_.template emplace<0>();
                    return;
                }
            }
        }
    }

    void clear() {
        switch (get_status()) {
            case SMALL: {
                src_.template emplace<0>();
                return;
            }
            case ALLOCATED: {
                std::get<0>(src_).reset();
            }
        }
    }

    void swap(vector<T>& that) {
        if (get_status() == EMPTY) {
            if (that.get_status() == EMPTY) {
                return;
            }
            if (that.get_status() == SMALL) {
                src_.template emplace<1>(std::get<1>(that.src_));
                that.src_.template emplace<0>();
                return;
            }
            if (that.get_status() == ALLOCATED) {
                src_.template emplace<0>(std::get<0>(that.src_));
                that.src_.template emplace<0>();
                return;
            }
        }
        if (get_status() == SMALL) {
            if (that.get_status() == EMPTY) {
                that.swap(*this);
                return;
            }
            if (that.get_status() == SMALL) {
                T tmp = std::get<1>(src_);
                src_.template emplace<1>(std::get<1>(that.src_));
                that.src_.template emplace<1>(tmp);
                return;
            }
            if (that.get_status() == ALLOCATED) {
                T tmp = std::get<1>(src_);
                src_.template emplace<0>(std::get<0>(that.src_));
                that.src_.template emplace<1>(tmp);
                return;
            }
        }
        if (get_status() == ALLOCATED) {
            if (that.get_status() == EMPTY || that.get_status() == SMALL) {
                that.swap(*this);
                return;
            }
            if (that.get_status() == ALLOCATED) {
                asp<T> tmp = std::get<0>(src_);
                src_.template emplace<0>(std::get<0>(that.src_));
                that.src_.template emplace<0>(tmp);
            }
        }
    }

    iterator begin() {
        return iterator(data());
    }

    iterator end() {
        return iterator(data() + size());
    }

    const_iterator begin() const {
        return const_iterator(data());
    }

    const_iterator end() const {
        return const_iterator(data());
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator cend() const {
        return end();
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator rcbegin() const {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator rcend() const {
        return const_reverse_iterator(cbegin());
    }

private:
    enum status {
        EMPTY,
        SMALL,
        ALLOCATED,
    };

    status get_status() const {
        if (src_.index() == 0) {
            return std::get<0>(src_).data_ == nullptr ? EMPTY : ALLOCATED;
        } else {
            return SMALL;
        }
    }

    void allocate(size_t cap = 2) {
        if (get_status() == SMALL) {
//            T tmp = std::get<1>(src_);

            asp<T> tmp(1, cap, &std::get<1>(src_));
            src_.template emplace<0>(tmp);

//            src_.template emplace<0>(1, cap);
//            std::get<0>(src_).get_size() = 1;
//            new (&std::get<0>(src_)[0]) T(tmp);
        } else {
            asp<T> tmp(0, cap);
            src_.template emplace<0>(tmp);
        }
    }

    void reallocate(size_t cap) {
        asp<T> tmp(std::get<0>(src_).get_size(), cap, std::get<0>(src_).get_data());
        src_.template emplace<0>(tmp);
    }

    void detach() {
        if (get_status() == ALLOCATED) {
//            asp<T> tmp(size(), capacity(), std::get<0>(src_).get_data());

        }
    }
};

template<typename T>
void swap(vector<T>& a, vector<T>& b) {
    a.swap(b);
}

#endif
