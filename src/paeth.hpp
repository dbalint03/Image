#pragma once

#include <cmath>

/// @brief Paeth Predictor function
/// @tparam T
/// @param a - left pixel
/// @param b - above pixel
/// @param c - upper left pixel
/// @return
template <typename T>
inline T paeth_predictor(const T a, const T b, const T c)
{
    const int p = static_cast<int>(a) + static_cast<int>(b) - static_cast<int>(c);
    const int pa = std::abs(p - static_cast<int>(a));
    const int pb = std::abs(p - static_cast<int>(b));
    const int pc = std::abs(p - static_cast<int>(c));
    if (pa <= pb && pa <= pc)
    {
        return a;
    }
    else if (pb <= pc)
    {
        return b;
    }
    else
    {
        return c;
    }
}
