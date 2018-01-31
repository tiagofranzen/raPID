#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>
#include <cstddef>

template <typename T, std::size_t n>
struct Filter {
        T values[n] = {};
        size_t marker = 0;

        T next(T value);
};

template <typename T, std::size_t n>
T Filter<T, n>::next(T value) {
    marker = (marker + 1) % n;
    values[marker] = value;
    T sum = 0;
    for (size_t index = 0; index < n; index++) {
        sum += values[index];
    }
    return sum / n;
}

#endif //FILTER_H
