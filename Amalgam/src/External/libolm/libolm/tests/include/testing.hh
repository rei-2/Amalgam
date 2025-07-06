#pragma once

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include "doctest.h"

template<class T>
struct ospan {
    ospan(T* data, std::size_t size) : data(data), size(size) {}

    template<std::size_t N>
    ospan(T (&array)[N]) : data(array), size(N) {}

    friend bool operator==(const ospan& a, const ospan& b) {
        if (a.size != b.size)
            return false;

        for (size_t i = 0; i < a.size; i++)
            if (a.data[i] != b.data[i])
                return false;

        return true;
    }

    friend std::ostream& operator<<(std::ostream& os, const ospan& v) {
        os << "sz:" << std::dec << v.size << " [" << std::hex;
        for (size_t i = 0; i < v.size; i++)
            os << (std::size_t)v.data[i];
        os << "]";
        return os;
    }

    T* data;
    std::size_t size;
};

template<class T>
auto with_size(T* data, std::size_t sz) -> ospan<T> { return ospan<T>(data, sz); }
template<class T, std::size_t N>
auto with_size(T (&data)[N]) -> ospan<T> { return ospan<T>(data, N); }

#define CHECK_EQ_SIZE(A, B, SIZE) CHECK_EQ(with_size(A, SIZE), with_size(B, SIZE))
