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
    int val;

    B(int val) : val(val) {};

    B(const B& that) : val(that.val) {
        if (val == 666) {
            throw std::exception();
        }
    }
};

TEST(vector, vector_ctor_failure_push_back) {
    vector<B> v;
    B cool1(1);
    B cool2(2);
    B badboy(666);
    B cool3(3);
    EXPECT_ANY_THROW(v.push_back(badboy));
    EXPECT_EQ(v.size(), 0);
    EXPECT_NO_THROW(v.push_back(cool1));
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v[0].val, 1);
    EXPECT_ANY_THROW(v.push_back(badboy));
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v[0].val, 1);
    EXPECT_NO_THROW(v.push_back(cool2));
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0].val, 1);
    EXPECT_EQ(v[1].val, 2);
    EXPECT_ANY_THROW(v.push_back(badboy));
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0].val, 1);
    EXPECT_EQ(v[1].val, 2);
    EXPECT_NO_THROW(v.push_back(cool3));
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0].val, 1);
    EXPECT_EQ(v[1].val, 2);
    EXPECT_EQ(v[2].val, 3);
}

TEST(vector, polygon) {
    vector<B> v;
    v.push_back(B(1));
    v.push_back(B(2));
    EXPECT_ANY_THROW(v.push_back(B(666)));
    EXPECT_EQ(v.size(), 2);
}

TEST(vector, detach) {
    vector<int> a;
    vector<int> b;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    b = a;
    a.push_back(4);
    EXPECT_NE(a.size(), b.size());
}

TEST(vector, push_back_itself) {
    vector<B> v;
    v.push_back(42);
    v.push_back(v.front());
    v.push_back(v.front());
    v.push_back(v.front());
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ(v[i].val, 42);
    }
}

TEST(vector, insert) {
    vector<B> v;
    auto it1 = v.insert(v.begin(), B(42));
    EXPECT_EQ(it1->val, 42);
    
    auto it2 = v.insert(v.end(), B(43));
    EXPECT_EQ(it2->val, 43);
    
    auto it3 = v.insert(v.begin(), B(41));
    EXPECT_EQ(it3->val, 41);
    
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.end() - v.begin(), v.size());
    for (auto it = v.begin(); it != v.end(); ++it) {
        EXPECT_EQ(it->val, 41 + (it - v.begin()));
    }
}