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

    vector() = default;

    vector(vector const& that) = default;

    template<typename InputIterator>
    vector(InputIterator first, InputIterator last) {
        vector tmp;
        for (auto it = first; it != last; ++it) {
            tmp.push_back(*it);
        }
        swap(tmp);
    }

    template<typename InputIterator>
    void assign(InputIterator first, InputIterator last) {
        vector tmp;
        for (auto it = first; it != last; ++it) {
            tmp.push_back(*it);
        }
        swap(tmp);
    }

    T& operator[](size_t i) {
        detach();
        if (get_status() == SMALL) {
            return std::get<1>(src_);
        } else {
            return std::get<0>(src_)[i];
        }
    }

    const T& operator[](size_t i) const {
        if (get_status() == SMALL) {
            return std::get<1>(src_);
        } else {
            return std::get<0>(src_)[i];
        }
    }

    T& front() {
        detach();
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
        detach();
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
        detach();
        switch (get_status()) {
            case EMPTY: {
                try {
                    src_.template emplace<1>(value);
                } catch (...) {
                    src_.template emplace<0>();
                    throw std::exception();
                }
                return;
            }
            case SMALL: {
                asp<T> tmp(std::get<1>(src_));

                new (&tmp[1]) T(value);
                ++tmp.get_size();

                src_.template emplace<0>(tmp);
                return;
            }
            case ALLOCATED: {
                size_t size = std::get<0>(src_).get_size();
                size_t cap = std::get<0>(src_).get_cap();
                if (size == cap) {
                    asp<T> tmp(std::get<0>(src_).get_size(), cap * 2, std::get<0>(src_).get_data());
                    new (&tmp[size]) T(value);
                    ++tmp.get_size();
                    src_.template emplace<0>(tmp);
                } else {
                    new (&std::get<0>(src_)[size]) T(value);
                    ++std::get<0>(src_).get_size();
                }
                return;
            }
        }
    }

    void pop_back() {
        detach();
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
        detach();
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
        return 0;
    }

    void reserve(size_t cap) {
        detach();
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
        detach();
        if (get_status() == ALLOCATED) {
            size_t size = std::get<0>(src_).get_size();
            reallocate(size);
        }
    }

    void resize(size_t new_size, const T& val = T()) {
        detach();
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
                case EMPTY: {
                    return;
                }
            }
        }
    }

    void clear() {
        detach();
        switch (get_status()) {
            case EMPTY: {
                return;
            }
            case SMALL: {
                src_.template emplace<0>();
                return;
            }
            case ALLOCATED: {
                std::get<0>(src_).reset();
                return;
            }
        }
    }

    void swap(vector<T>& that) {
        detach();
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
        detach();
        return iterator(data());
    }

    iterator end() {
        detach();
        return iterator(data() + size());
    }

    const_iterator begin() const {
        return const_iterator(data());
    }

    const_iterator end() const {
        return const_iterator(data() + size());
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator cend() const {
        return end();
    }

    reverse_iterator rbegin() {
        detach();
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        detach();
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

    iterator insert(const_iterator pos, const T& val) {
        detach();
        if (pos == end()) {
            push_back(val);
            return end() - 1;
        } else {
            switch (get_status()) {
                case SMALL: {
                    asp<T> tmp(val);
                    new (&tmp[1]) T(std::get<1>(src_));
                    ++tmp.get_size();

                    src_.template emplace<0>(tmp);
                    return begin();
                }
                case ALLOCATED: {
                    size_t size = this->size();
                    size_t cap = capacity();
                    if (size == cap) {
                        cap *= 2;
                    }
                    asp<T> tmp(0, cap, nullptr);

                    size_t i = 0;
                    auto it = cbegin();
                    for (; it != pos; ++it, ++i) {
                        new (&tmp[i]) T(*it);
                        ++tmp.get_size();
                    }

                    iterator ret(tmp.get_data() + i);
                    new (&tmp[i]) T(val);
                    ++tmp.get_size();
                    ++i;

                    for (; it != cend(); ++it, ++i) {
                        new (&tmp[i]) T(*it);
                        ++tmp.get_size();
                    }

                    std::get<0>(src_) = tmp;
                    return ret;
                }
                default: {
                    throw std::exception();
                }
            }
        }
    }

    iterator erase(const_iterator pos) {
        return erase(pos, pos + 1);
    }

    iterator erase(const_iterator first, const_iterator last) {
        detach();
        if (first == last && first + 1 == end()) {
            pop_back();
            return end();
        } else {
            switch (get_status()) {
                case ALLOCATED: {
                    size_t cap = capacity();
                    asp<T> tmp(0, cap, nullptr);

                    size_t i = 0;
                    auto it = cbegin();
                    for (; it != first; ++it, ++i) {
                        new (&tmp[i]) T(*it);
                        ++tmp.get_size();
                    }

                    iterator ret(tmp.get_data() + i);
                    for (; it != last; ++it);

                    for (; it != cend(); ++it, ++i) {
                        new (&tmp[i]) T(*it);
                        ++tmp.get_size();
                    }

                    std::get<0>(src_) = tmp;
                    return ret;
                }
                default: {
                    throw std::exception();
                }
            }
        }
    }

    friend bool operator==(const vector& a, const vector& b) {
        if (a.size() != b.size()) {
            return false;
        }
        auto ita = a.begin();
        auto itb = b.begin();
        for (; ita != a.end(); ++ita, ++itb) {
            if (*ita != *itb) {
                return false;
            }
        }
        return true;
    }

    friend bool operator!=(const vector& a, const vector& b) {
        return !(a == b);
    }

    friend bool operator<(const vector& a, const vector& b) {
        auto a_begin = a.begin();
        auto b_begin = b.begin();

        auto a_end = a.end();
        auto b_end = b.end();

        for (; (a_begin != a_end) && (b_begin != b_end); ++a_begin, ++b_begin) {
            if (*a_begin < *b_begin) {
                return true;
            }
            if (*b_begin < *a_begin) {
                return false;
            }
        }

        return (a_begin == a_end) && (b_begin != b_end);
    }

    friend bool operator>(const vector& a, const vector& b) {
        return b < a;
    }

    friend bool operator<=(const vector& a, const vector& b) {
        return !(b < a);
    }

    friend bool operator>=(const vector& a, const vector& b) {
        return !(a < b);
    }

private:
    std::variant<asp<T>, T> src_;

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
            asp<T> tmp(std::get<1>(src_));
            src_.template emplace<0>(tmp);
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
        if (get_status() == ALLOCATED && !std::get<0>(src_).unique()) {
            asp<T> tmp(size(), capacity(), std::get<0>(src_).get_data());
            std::get<0>(src_) = tmp;
        }
    }
};

template<typename T>
void swap(vector<T>& a, vector<T>& b) {
    if (&a != &b) {
        a.swap(b);
    }
}



#endif
