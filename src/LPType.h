#ifndef LPTYPE_H
#define LPTYPE_H

#include <png.h>
#include <algorithm>

template<typename T>
static inline void clamp(T& value, int min_value, int max_value)
{
    value = static_cast<T>(
            std::min(
                std::max(
                    static_cast<int>(value),
                    min_value),
                max_value));
}

#include <boost/shared_array.hpp>

typedef boost::shared_array<unsigned char*> rows_t;
typedef boost::shared_array<unsigned char> buffer_t;
typedef boost::shared_array<png_color> palette_t;
typedef boost::shared_array<unsigned char> trans_t;

#endif