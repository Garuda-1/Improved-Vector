#include "asp.hpp"
#include "gtest/gtest.h"

TEST(asp, alpha) {
    int arr[5] = {0, 1, 2, 3, 4};

    asp<int> test0(5, 8, arr);
    asp test1 = test0;

    for (size_t i = 0; i < 5; i++) {
        EXPECT_EQ(arr[i], test0[i]);
        EXPECT_EQ(arr[i], test1[i]);
    }

    test1[2] = 101;
    EXPECT_EQ(test0[2], test1[2]);
    test0[2] = 2;


    for (size_t i = 0; i < 5; i++) {
        EXPECT_EQ(arr[i], test0[i]);
        EXPECT_EQ(arr[i], test1[i]);
    }

    int arr1[3] = {2, 1, 0};
    test1.reset(3, 4, arr1);

    for (size_t i = 0; i < 5; i++) {
        EXPECT_EQ(arr[i], test0[i]);
    }

    for (size_t i = 0; i < 3; i++) {
        EXPECT_EQ(arr1[i], test1[i]);
    }

    test0.reset(test1.get_size(), test1.get_cap(), test1.get_data());

    for (size_t i = 0; i < 3; i++) {
        EXPECT_EQ(test0[i], test1[i]);
    }
}