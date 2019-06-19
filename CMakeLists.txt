cmake_minimum_required(VERSION 3.13)
project(exam_vector)

#if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Wall -pedantic")
#    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -fsanitize=address -fsanitize=leak -Wall -std=c++11 -pedantic -D_GLIBCXX_DEBUG")
#endif()

set(CMAKE_CXX_STANDARD 17)

add_executable(
        asp-testing
        gtest/gtest-all.cc
        gtest/gtest.h
        gtest/gtest-asp.cc
        asp.hpp
        asp-tests.cpp
        vector-tests.cpp)

add_executable(
        vector-testing
        gtest/gtest-all.cc
        gtest/gtest.h
        gtest/gtest-vector.cc
        asp.hpp
        vector.hpp
        vector-tests.cpp
        vector_iterator.hpp)

add_executable(
        sorokin-testing

        gtest/gtest-all.cc
        gtest/gtest.h
        gtest/gtest-vector.cc

        asp.hpp
        vector.hpp

        counted.cpp
        counted.h
        fault_injection.cpp
        fault_injection.h

        vector_testing.cpp
)

target_link_libraries(asp-testing pthread)
target_link_libraries(vector-testing pthread)
target_link_libraries(sorokin-testing -lpthread)