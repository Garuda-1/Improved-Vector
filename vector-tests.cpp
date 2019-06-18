#include "vector.hpp"
#include "gtest/gtest.h"

template<typename T>
static size_t limit() {
    return std::max(2 * sizeof(void*), sizeof(void*) + sizeof(T));
}

TEST(vector, size) {
    typedef std::string T;
    vector<T> v;
    EXPECT_TRUE(sizeof(v) <= limit<T>()) << sizeof(v) << " " << sizeof(v.src_) << " " << sizeof(T) << " " << sizeof(asp<T>);
}

TEST(vector, access) {
    typedef int T;
    vector<T> v;

    EXPECT_EQ(v.size(), 0);

    for (int i = 1; i <= 5; ++i) {
        v.push_back(i);
        EXPECT_EQ(v.size(), i);
        EXPECT_TRUE(sizeof(v) <= limit<T>());
        for (size_t j = 0; j < v.size(); ++j) {
            EXPECT_EQ(v[j], j + 1);
        }
    }

    v.front() = 101;
    v.back() = 666;
    EXPECT_EQ(v[0], 101);
    EXPECT_EQ(v[4], 666);

    v.front() = 1;
    v.back() = 5;

    for (int i = 5; i-- > 0;) {
        v.pop_back();
        EXPECT_EQ(v.size(), i);
        EXPECT_TRUE(sizeof(v) <= limit<T>());
        for (size_t j = 0; j < v.size(); ++j) {
            EXPECT_EQ(v[j], j + 1);
        }
    }
}

TEST(vector, modifiers) {
    typedef int T;
    vector<T> v;

    EXPECT_EQ(v.size(), 0);

    v.resize(4, 1);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(v[i], 1);
    }

    v.resize(8, 2);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(v[i], 1);
    }
    for (size_t i = 4; i < 8; ++i) {
        EXPECT_EQ(v[i], 2);
    }

    v.resize(12, 3);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(v[i], 1);
    }
    for (size_t i = 4; i < 8; ++i) {
        EXPECT_EQ(v[i], 2);
    }
    for (size_t i = 8; i < 12; ++i) {
        EXPECT_EQ(v[i], 3);
    }

    size_t prev_cap = v.capacity();
    v.resize(3);
    EXPECT_EQ(prev_cap, v.capacity());
    for (size_t i = 0; i < 3; ++i) {
        EXPECT_EQ(v[i], 1);
    }
    v.front() = 101;

    prev_cap = v.capacity();
    v.pop_back();
    v.pop_back();

    EXPECT_EQ(prev_cap, v.capacity());
    EXPECT_EQ(v.front(), 101);
    EXPECT_EQ(v.front(), v[0]);

    v.reserve(20);
    v.resize(20, 5);

    EXPECT_EQ(v[0], 101);
    for (size_t i = 1; i < 20; ++i) {
        EXPECT_EQ(v[i], 5);
    }

    v.resize(5);

    EXPECT_EQ(v[0], 101);
    for (size_t i = 1; i < 5; ++i) {
        EXPECT_EQ(v[i], 5);
    }

    v.shrink_to_fit();
    EXPECT_EQ(v.size(), v.capacity());
}

struct B {
    bool val;

    B(bool val) : val(val) {};

    B(const B&) {
        if (val) {
            throw std::exception();
        }
    }
};

TEST(vector, wew) {
    vector<B> v;
    B cool1(false);
    B cool2(false);
    B badboy(true);
    B cool3(false);
    EXPECT_NO_THROW(v.push_back(cool1));
    EXPECT_NO_THROW(v.push_back(cool2));
    EXPECT_ANY_THROW(v.push_back(badboy));
    EXPECT_EQ(v.size(), 2);
    EXPECT_NO_THROW(v.push_back(cool3));
    EXPECT_EQ(v.size(), 3);
}