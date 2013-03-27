#ifndef REDUCECOLOR_H
#define REDUCECOLOR_H

#include <iostream>
#include <cstring>
#include "LPType.h"
#include "Image.h"


template <size_t A>
class ColorReducer
{
private:
    rows_t _src;
    rows_t _dest;
    buffer_t _rawsrc;
    buffer_t _rawdest;
    size_t _width, _height;
    unsigned _R, _G, _B;

    inline void get(rows_t& img, size_t x, size_t y, int& r, int& g, int& b, int& a)
    {
        clamp(x, 0, _width - 1);
        clamp(y, 0, _height - 1);

        r = static_cast<int>(img[y][4 * x]);
        g = static_cast<int>(img[y][4 * x + 1]);
        b = static_cast<int>(img[y][4 * x + 2]);
        a = static_cast<int>(img[y][4 * x + 3]);
    };

    inline void set(rows_t& img, size_t x, size_t y, int r, int g, int b, int a)
    {
        clamp(x, 0, _width - 1);
        clamp(y, 0, _height - 1);
        clamp(r, 0, 255);
        clamp(g, 0, 255);
        clamp(b, 0, 255);
        clamp(a, 0, 255);

        img[y][4 * x]     = static_cast<unsigned char>(r);
        img[y][4 * x + 1] = static_cast<unsigned char>(g);
        img[y][4 * x + 2] = static_cast<unsigned char>(b);
        img[y][4 * x + 3] = static_cast<unsigned char>(a);
    };

public:
    explicit ColorReducer(size_t width, size_t height, unsigned R, unsigned G, unsigned B)
        : _width(width), _height(height), _R(R), _G(G), _B(B)
    {
        _rawsrc.reset(new unsigned char[4 * width * height]);
        _rawdest.reset(new unsigned char[4 * width * height]);
        _src.reset(new unsigned char*[height]);
        _dest.reset(new unsigned char*[height]);
        for (size_t i = 0; i < _height; ++i)
        {
            _src[i] = _rawsrc.get() + (i * width * 4);
            _dest[i] = _rawdest.get() + (i * width * 4);
        }
    };

    void process(rows_t src, bool preview = true)
    {
        Image::copy_4_to_4(_width, _height, src, _src);
        const int MaxR = (1 << _R) - 1;
        const int MaxG = (1 << _G) - 1;
        const int MaxB = (1 << _B) - 1;
        const int MaxA = (1 << A) - 1;
        const int OriginalMax = (1 << 8) - 1;
  
        float f_r, f_g, f_b, f_a;
        if (preview)
        {
            f_r = 255.0 / (256 - (1 << (8 - _R)));
            f_g = 255.0 / (256 - (1 << (8 - _G)));
            f_b = 255.0 / (256 - (1 << (8 - _B)));
            f_a = 255.0 / (256 - (1 << (8 - A)));
        }
        else
        {
            f_r = 1;
            f_g = 1;
            f_b = 1;
            f_a = 1;
        }
        for (size_t y = 0; y < _height; ++y)
        {
            for (size_t x = 0; x < _width; ++x)
            {
                int old_r, old_g, old_b, old_a;
                get(_src, x, y, old_r, old_g, old_b, old_a);
            
                unsigned char new_r = (static_cast<int>(old_r) * MaxR / OriginalMax) << (8 - _R);
                unsigned char new_g = (static_cast<int>(old_g) * MaxG / OriginalMax) << (8 - _G);
                unsigned char new_b = (static_cast<int>(old_b) * MaxB / OriginalMax) << (8 - _B);
                unsigned char new_a = (static_cast<int>(old_a) * MaxA / OriginalMax) << (8 - A);

                set(_dest, x, y, static_cast<int>(f_r * new_r), static_cast<int>(f_g * new_g), static_cast<int>(f_b * new_b), static_cast<int>(f_a * new_a));

                int dr = std::abs(old_r - new_r);
                int dg = std::abs(old_g - new_g);
                int db = std::abs(old_b - new_b);
                int da = std::abs(old_a - new_a);

                int r, g, b, a;
                get(_src, x + 1, y, r, g, b, a);
                r += dr * 7 / 16;
                g += dg * 7 / 16;
                b += db * 7 / 16;
                a += da * 7 / 16;
                set(_src, x + 1, y, r, g, b, a);

                get(_src, x - 1, y + 1, r, g, b, a);
                r += dr * 3 / 16;
                g += dg * 3 / 16;
                b += db * 3 / 16;
                a += da * 3 / 16;
                set(_src, x - 1, y + 1, r, g, b, a);

                get(_src, x, y + 1, r, g, b, a);
                r += dr * 5 / 16;
                g += dg * 5 / 16;
                b += db * 5 / 16;
                a += da * 5 / 16;
                set(_src, x, y + 1, r, g, b, a);

                get(_src, x + 1, y + 1, r, g, b, a);
                r += dr * 1 / 16;
                g += dg * 1 / 16;
                b += db * 1 / 16;
                a += da * 1 / 16;
                set(_src, x + 1, y + 1, r, g, b, a);
            }
        }
    };

    buffer_t buffer() {
        return _rawdest;
    };
};

template<>
class ColorReducer<0>
{
private:
    rows_t _src;
    rows_t _dest;
    buffer_t _rawsrc;
    buffer_t _rawdest;
    size_t _width, _height;
    unsigned _R, _G, _B;

    inline void get(rows_t& img, size_t x, size_t y, int& r, int& g, int& b)
    {
        clamp(x, 0, _width - 1);
        clamp(y, 0, _height - 1);

        r = static_cast<int>(img[y][3 * x]);
        g = static_cast<int>(img[y][3 * x + 1]);
        b = static_cast<int>(img[y][3 * x + 2]);
    };

    inline void set(rows_t& img, size_t x, size_t y, int r, int g, int b)
    {
        clamp(x, 0, _width - 1);
        clamp(y, 0, _height - 1);
        clamp(r, 0, 255);
        clamp(g, 0, 255);
        clamp(b, 0, 255);

        img[y][3 * x]     = static_cast<unsigned char>(r);
        img[y][3 * x + 1] = static_cast<unsigned char>(g);
        img[y][3 * x + 2] = static_cast<unsigned char>(b);
    };
public:
    explicit ColorReducer(size_t width, size_t height, unsigned R, unsigned G, unsigned B)
        : _width(width), _height(height), _R(R), _G(G), _B(B)
    {
        _rawsrc.reset(new unsigned char[3 * width * height]);
        _rawdest.reset(new unsigned char[3 * width * height]);
        _src.reset(new unsigned char*[height]);
        _dest.reset(new unsigned char*[height]);
        for (size_t i = 0; i < height; ++i)
        {
            _src[i] = _rawsrc.get() + (i * width * 3);
            _dest[i] = _rawdest.get() + (i * width * 3);
        }
    };

    void process(rows_t src, bool hasAlphaChannel, bool preview = true)
    {
        if (hasAlphaChannel)
        {
            Image::copy_4_to_3(_width, _height, src, _src);
        }
        else
        {
            Image::copy_3_to_3(_width, _height, src, _src);
        }
        const int MaxR = (1 << _R) - 1;
        const int MaxG = (1 << _G) - 1;
        const int MaxB = (1 << _B) - 1;
        const int OriginalMax = (1 << 8) - 1;

        float f_r, f_g, f_b;
        if (preview)
        {
            f_r = 255.0 / (256 - (1 << (8 - _R)));
            f_g = 255.0 / (256 - (1 << (8 - _G)));
            f_b = 255.0 / (256 - (1 << (8 - _B)));
        }
        else
        {
            f_r = 1;
            f_g = 1;
            f_b = 1;
        }

        for (size_t y = 0; y < _height; ++y)
        {
            for (size_t x = 0; x < _width; ++x)
            {
                int old_r, old_g, old_b;
                get(_src, x, y, old_r, old_g, old_b);
                
                unsigned char new_r = (static_cast<int>(old_r) * MaxR / OriginalMax) << (8 - _R);
                unsigned char new_g = (static_cast<int>(old_g) * MaxG / OriginalMax) << (8 - _G);
                unsigned char new_b = (static_cast<int>(old_b) * MaxB / OriginalMax) << (8 - _B);

                set(_dest, x, y, static_cast<int>(f_r * new_r), static_cast<int>(f_g * new_g), static_cast<int>(f_b * new_b));

                int dr = std::abs(old_r - new_r);
                int dg = std::abs(old_g - new_g);
                int db = std::abs(old_b - new_b);

                int r, g, b;
                get(_src, x + 1, y, r, g, b);
                r += dr * 7 / 16;
                g += dg * 7 / 16;
                b += db * 7 / 16;
                set(_src, x + 1, y, r, g, b);

                get(_src, x - 1, y + 1, r, g, b);
                r += dr * 3 / 16;
                g += dg * 3 / 16;
                b += db * 3 / 16;
                set(_src, x - 1, y + 1, r, g, b);

                get(_src, x, y + 1, r, g, b);
                r += dr * 5 / 16;
                g += dg * 5 / 16;
                b += db * 5 / 16;
                set(_src, x, y + 1, r, g, b);

                get(_src, x + 1, y + 1, r, g, b);
                r += dr * 1 / 16;
                g += dg * 1 / 16;
                b += db * 1 / 16;
                set(_src, x + 1, y + 1, r, g, b);
            }
        }
    };
    buffer_t buffer() {
        return _rawdest;
    };
};

template<unsigned A>
buffer_t reduce_color(Image& image, unsigned R, unsigned G, unsigned B, bool hasAlphaChannel, bool preview)
{
    ColorReducer<A> reducer(image.width(), image.height(), R, G, B);
    reducer.process(image.image(), preview);
    return reducer.buffer();
}


template<>
buffer_t reduce_color<0>(Image& image, unsigned R, unsigned G, unsigned B, bool hasAlphaChannel, bool preview)
{
    ColorReducer<0> reducer(image.width(), image.height(), R, G, B);
    reducer.process(image.image(), hasAlphaChannel, preview);
    return reducer.buffer();
}

#endif
