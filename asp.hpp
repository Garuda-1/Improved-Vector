#ifndef EXAM_VECTOR_ASP_HPP
#define EXAM_VECTOR_ASP_HPP

#include <cstdio>
#include <exception>

template<typename T>
struct asp {
    char* data_;

    explicit asp(size_t size = 0, size_t cap = 0, T* data = nullptr) : data_(nullptr) {
        construct_chunk(size, cap, data);
    }

    explicit asp(const T& data) {
        construct_chunk(data);
    }

    asp(const asp<T>& that) noexcept : data_(nullptr) {
        attach(that.data_);
    }

    ~asp() {
        detach();
    }

    asp& operator=(const asp& that) {
        if (data_ != nullptr && that.data_ != nullptr && data_ == that.data_) {
            return *this;
        }
        detach();
        attach(that.data_);
        return *this;
    }

    void reset(size_t size = 0, size_t cap = 0, T* data = nullptr) {
        detach();
        construct_chunk(size, cap, data);
    }

    T& operator*() {
        return *(get_data());
    }

    T* operator->() {
        return (data_ == nullptr) ? nullptr : get_data();
    }

    T& operator[](size_t i) {
        return get_data()[i];
    }

    const T& operator[](size_t i) const {
        return get_data()[i];
    }

    bool unique() {
        return (data_ == nullptr) ? false : (get_ref() == 1);
    }

    size_t& get_ref() {
        return *reinterpret_cast<size_t*>(data_);
    }

    size_t& get_size() {
        return *reinterpret_cast<size_t*>(data_ + sizeof(size_t));
    }

    size_t get_size() const {
        return *reinterpret_cast<size_t*>(data_ + sizeof(size_t));
    }

    size_t& get_cap() {
        return *reinterpret_cast<size_t*>(data_ + 2 * sizeof(size_t));
    }

    size_t get_cap() const {
        return *reinterpret_cast<size_t*>(data_ + 2 * sizeof(size_t));
    }

    T* get_data() {
        return reinterpret_cast<T*>(data_ + 3 * sizeof(size_t));
    }

    const T* get_data() const {
        return reinterpret_cast<T*>(data_ + 3 * sizeof(size_t));
    }

private:
    void detach() {
        if (data_ != nullptr) {
            --get_ref();
            if (get_ref() == 0) {
                for (size_t i = 0; i < get_size(); ++i) {
                    get_data()[i].~T();
                }
                operator delete[] (data_);
            }
        }
    }

    void attach(char* that_data) noexcept {
        if (that_data == nullptr) {
            data_ = nullptr;
        } else {
            data_ = that_data;
            ++get_ref();
        }
    }

    void construct_chunk(size_t size, size_t cap, T* data) {
        if (cap == 0) {
            data_ = nullptr;
        } else {
            data_ = static_cast<char*>(operator new[] (3 * sizeof(size_t) + cap * sizeof(T)));
            get_ref() = 1;
            get_size() = size;
            get_cap() = cap;
            if (data != nullptr) {
                for (size_t i = 0; i < size; ++i) {
                    try {
                        new (get_data() + i) T(*(data + i));
                    } catch (...) {
                        for (size_t j = 0; j < i; j++) {
                            (get_data() + j)->~T();
                        }
                        operator delete[](data_);
                        data_ = nullptr;
                        throw std::exception();
                    }
                }
            }
        }
    }

    void construct_chunk(const T& data) {
        data_ = static_cast<char*>(operator new[] (3 * sizeof(size_t) + 2 * sizeof(T)));
        get_ref() = 1;
        get_size() = 1;
        get_cap() = 2;
        try {
            new (get_data()) T(data);
        } catch (...) {
            operator delete[](data_);
            data_ = nullptr;
            throw std::exception();
        }
    }
};

#endif
