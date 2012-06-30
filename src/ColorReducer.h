#ifndef COLORREDUCER_H
#define COLORREDUCER_H

#include <cmath>
#include <cstdio>
#include <cmemory>
#include <iostream>

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

template <unsigned A>
class ColorReducer
{
public:
    explicit ColorReducer(size_t width, size_t height, unsigned R, unsigned G, unsigned B);
    virtual ~ColorReducer();

    operator void*() const { return _valid ? const_cast<ColorReducer*>(this) : 0; }
    bool operator!() const { return !_valid; }

    void process(unsigned char** src, bool preview = true);
    unsigned char** rawimage() const { return _dest; };

private:
    unsigned char** _src;
    unsigned char** _dest;
    size_t _width, _height;
    unsigned _R, _G, _B;
    bool _valid;

    void destroy();
    inline void get(unsigned char** img, size_t x, size_t y, int& r, int& g, int& b, int& a)
    {
        clamp(x, 0, _width - 1);
        clamp(y, 0, _height - 1);

        r = static_cast<int>(img[y][4 * x]);
        g = static_cast<int>(img[y][4 * x + 1]);
        b = static_cast<int>(img[y][4 * x + 2]);
        a = static_cast<int>(img[y][4 * x + 3]);
    };

    inline void set(unsigned char** img, size_t x, size_t y, int r, int g, int b, int a)
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
};


template <>
class ColorReducer<0>
{
public:
    explicit ColorReducer(size_t width, size_t height, unsigned R, unsigned G, unsigned B);
    virtual ~ColorReducer();

    operator void*() const { return _valid ? const_cast<ColorReducer*>(this) : 0; }
    bool operator!() const { return !_valid; }
    unsigned char** rawimage() const { return _dest; };

    void process(unsigned char** src, bool preview = true);
    bool write(const char* filepath);

private:
    unsigned char** _src;
    unsigned char** _dest;
    unsigned char* _rawimage;
    size_t _width, _height;
    unsigned _R, _G, _B;
    bool _valid;

    void destroy();
    inline void get(unsigned char**, size_t x, size_t y, int& r, int& g, int& b)
    {
        clamp(x, 0, _width - 1);
        clamp(y, 0, _height - 1);

        r = static_cast<int>(img[y][3 * x]);
        g = static_cast<int>(img[y][3 * x + 1]);
        b = static_cast<int>(img[y][3 * x + 2]);
    };

    inline void set(unsigned char**, size_t x, size_t y, int r, int g, int b)
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
};


template <unsigned A>
ColorReducer<A>::ColorReducer(size_t width, size_t height, unsigned R, unsigned G, unsigned B)
    : _png(0), _info(0), _dest(0), _width(width), _height(height), _R(R), _G(G), _B(B), _valid(false)
{
    _dest = new unsigned char*[height];
    _src = new unsigned char*[height];
    for (int i = 0; i < height; ++i)
    {
        _dest[i] = new unsigned char[width * 4];
        _src[i] = new unsigned char[width * 4];
    }
    _valid = true;
};

ColorReducer<0>::ColorReducer(size_t width, size_t height, unsigned R, unsigned G, unsigned B)
    : _png(0), _info(0), _dest(0), _width(width), _height(height), _R(R), _G(G), _B(B), _valid(false)
{
    _dest = new unsigned char*[height];
    _src = new unsigned char*[height];
    for (int i = 0; i < height; ++i)
    {
        _dest[i] = new unsigned char[width * 3];
        _src[i] = new unsigned char[width * 3];
    }
    _valid = true;
};

template <unsigned A>
void ColorReducer<A>::process(unsigned char** src, bool preview)
{
    for (int i = 0; i < height; ++i)
    {
        memcpy(_src[i], src[i], _width * 4);
    }
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

void ColorReducer<0>::process(unsigned charpp src, bool preview)
{
    for (int i = 0; i < height; ++i)
    {
        memcpy(_src[i], src[i], _width * 3);
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

    for (int y = 0; y < _height; ++y)
    {
        for (int x = 0; x < _width; ++x)
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

template <unsigned A>
ColorReducer<A>::~ColorReducer()
{
    destroy();
};

ColorReducer<0>::~ColorReducer()
{
    destroy();
};

template <unsigned A>
void ColorReducer<A>::destroy()
{
    png_destroy_write_struct(&_png, &_info);
    for (int i = 0; i < _height; ++i)
    {
        delete[] _dest[i];
        delete[] _src[i];
    }
    delete _dest;
    delete _src;
};

void ColorReducer<0>::destroy()
{
    png_destroy_write_struct(&_png, &_info);
    for (int i = 0; i < _height; ++i)
    {
        delete[] _dest[i];
        delete[] _src[i];
    }
    delete _dest;
    delete _src;
}

#endif
