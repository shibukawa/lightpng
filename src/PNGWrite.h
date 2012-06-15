#ifndef PNGWRITE_H
#define PNGWRITE_H

#include <png.h>
#include <cmath>
#include <cstdio>
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
class PNGWrite
{
public:
    explicit PNGWrite(size_t width, size_t height, unsigned R, unsigned G, unsigned B);
    virtual ~PNGWrite();

    operator void*() const { return _valid ? const_cast<PNGWrite*>(this) : 0; }
    bool operator!() const { return !_valid; }

    void process(png_bytepp src, bool preview = true);
    bool write(const char* filepath);

private:
    png_structp _png;
    png_infop _info, _end;

    png_bytepp _dest;
    size_t _width, _height;
    unsigned _R, _G, _B;
    bool _valid;

    void destroy();
    inline void get(png_bytepp img, size_t x, size_t y, int& r, int& g, int& b, int& a)
    {
        clamp(x, 0, _width - 1);
        clamp(y, 0, _height - 1);

        r = static_cast<int>(img[y][4 * x]);
        g = static_cast<int>(img[y][4 * x + 1]);
        b = static_cast<int>(img[y][4 * x + 2]);
        a = static_cast<int>(img[y][4 * x + 3]);
    };

    inline void set(png_bytepp img, size_t x, size_t y, int r, int g, int b, int a)
    {
        clamp(x, 0, _width - 1);
        clamp(y, 0, _height - 1);
        clamp(r, 0, 255);
        clamp(g, 0, 255);
        clamp(b, 0, 255);
        clamp(a, 0, 255);

        img[y][4 * x]     = static_cast<png_byte>(r);
        img[y][4 * x + 1] = static_cast<png_byte>(g);
        img[y][4 * x + 2] = static_cast<png_byte>(b);
        img[y][4 * x + 3] = static_cast<png_byte>(a);
    };
};

template <>
class PNGWrite<0>
{
public:
    explicit PNGWrite(size_t width, size_t height, unsigned R, unsigned G, unsigned B);
    virtual ~PNGWrite();

    operator void*() const { return _valid ? const_cast<PNGWrite*>(this) : 0; }
    bool operator!() const { return !_valid; }

    void process(png_bytepp src, bool preview = true);
    bool write(const char* filepath);

private:
    png_structp _png;
    png_infop _info, _end;

    png_bytepp _dest;
    size_t _width, _height;
    unsigned _R, _G, _B;
    bool _valid;

    void destroy();
    inline void get(png_bytepp img, size_t x, size_t y, int& r, int& g, int& b)
    {
        clamp(x, 0, _width - 1);
        clamp(y, 0, _height - 1);

        r = static_cast<int>(img[y][3 * x]);
        g = static_cast<int>(img[y][3 * x + 1]);
        b = static_cast<int>(img[y][3 * x + 2]);
    };

    inline void set(png_bytepp img, size_t x, size_t y, int r, int g, int b)
    {
        clamp(x, 0, _width - 1);
        clamp(y, 0, _height - 1);
        clamp(r, 0, 255);
        clamp(g, 0, 255);
        clamp(b, 0, 255);

        img[y][3 * x]     = static_cast<png_byte>(r);
        img[y][3 * x + 1] = static_cast<png_byte>(g);
        img[y][3 * x + 2] = static_cast<png_byte>(b);
    };
};


template <unsigned A>
PNGWrite<A>::PNGWrite(size_t width, size_t height, unsigned R, unsigned G, unsigned B)
    : _png(0), _info(0), _dest(0), _width(width), _height(height), _R(R), _G(G), _B(B), _valid(false)
{
    _dest = new png_bytep[height];
    for (int i = 0; i < height; ++i)
    {
        _dest[i] = new png_byte[width * 4];
    }
    _valid = true;
};

PNGWrite<0>::PNGWrite(size_t width, size_t height, unsigned R, unsigned G, unsigned B)
    : _png(0), _info(0), _dest(0), _width(width), _height(height), _R(R), _G(G), _B(B), _valid(false)
{
    _dest = new png_bytep[height];
    for (int i = 0; i < height; ++i)
    {
        _dest[i] = new png_byte[width * 3];
    }
    _valid = true;
};

template <unsigned A>
void PNGWrite<A>::process(png_bytepp src, bool preview)
{
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
            get(src, x, y, old_r, old_g, old_b, old_a);
            
            png_byte new_r = (static_cast<int>(old_r) * MaxR / OriginalMax) << (8 - _R);
            png_byte new_g = (static_cast<int>(old_g) * MaxG / OriginalMax) << (8 - _G);
            png_byte new_b = (static_cast<int>(old_b) * MaxB / OriginalMax) << (8 - _B);
            png_byte new_a = (static_cast<int>(old_a) * MaxA / OriginalMax) << (8 - A);
            set(_dest, x, y, f_r * new_r, f_g * new_g, f_b * new_b, f_a * new_a);

            int dr = std::abs(old_r - new_r);
            int dg = std::abs(old_g - new_g);
            int db = std::abs(old_b - new_b);
            int da = std::abs(old_a - new_a);

            int r, g, b, a;
            get(src, x + 1, y, r, g, b, a);
            r += dr * 7 / 16;
            g += dg * 7 / 16;
            b += db * 7 / 16;
            a += da * 7 / 16;
            set(src, x + 1, y, r, g, b, a);

            get(src, x - 1, y + 1, r, g, b, a);
            r += dr * 3 / 16;
            g += dg * 3 / 16;
            b += db * 3 / 16;
            a += da * 3 / 16;
            set(src, x - 1, y + 1, r, g, b, a);

            get(src, x, y + 1, r, g, b, a);
            r += dr * 5 / 16;
            g += dg * 5 / 16;
            b += db * 5 / 16;
            a += da * 5 / 16;
            set(src, x, y + 1, r, g, b, a);

            get(src, x + 1, y + 1, r, g, b, a);
            r += dr * 1 / 16;
            g += dg * 1 / 16;
            b += db * 1 / 16;
            a += da * 1 / 16;
            set(src, x + 1, y + 1, r, g, b, a);
        }
    }
};

void PNGWrite<0>::process(png_bytepp src, bool preview)
{
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
            get(src, x, y, old_r, old_g, old_b);
            
            png_byte new_r = (static_cast<int>(old_r) * MaxR / OriginalMax) << (8 - _R);
            png_byte new_g = (static_cast<int>(old_g) * MaxG / OriginalMax) << (8 - _G);
            png_byte new_b = (static_cast<int>(old_b) * MaxB / OriginalMax) << (8 - _B);

            set(_dest, x, y, f_r * new_r, f_g * new_g, f_b * new_b);

            int dr = std::abs(old_r - new_r);
            int dg = std::abs(old_g - new_g);
            int db = std::abs(old_b - new_b);

            int r, g, b;
            get(src, x + 1, y, r, g, b);
            r += dr * 7 / 16;
            g += dg * 7 / 16;
            b += db * 7 / 16;
            set(src, x + 1, y, r, g, b);

            get(src, x - 1, y + 1, r, g, b);
            r += dr * 3 / 16;
            g += dg * 3 / 16;
            b += db * 3 / 16;
            set(src, x - 1, y + 1, r, g, b);

            get(src, x, y + 1, r, g, b);
            r += dr * 5 / 16;
            g += dg * 5 / 16;
            b += db * 5 / 16;
            set(src, x, y + 1, r, g, b);

            get(src, x + 1, y + 1, r, g, b);
            r += dr * 1 / 16;
            g += dg * 1 / 16;
            b += db * 1 / 16;
            set(src, x + 1, y + 1, r, g, b);
        }
    }
};

template <unsigned A>
bool PNGWrite<A>::write(const char* filepath)
{
    FILE *fp = std::fopen(filepath, "wb");
    if (!fp)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return false;
    }
    _png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!_png)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return false;
    }
    _info = png_create_info_struct(_png);
    if (!_info)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return false;
    }
    png_init_io(_png, fp);
    png_set_compression_level(_png, Z_BEST_COMPRESSION);
    png_set_IHDR(_png, _info, _width, _height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(_png, _info);
    png_write_image(_png, _dest);
    png_write_png(_png, _info, PNG_TRANSFORM_IDENTITY, NULL);
    png_write_end(_png, _info);
    return true;
};

bool PNGWrite<0>::write(const char* filepath)
{
    FILE *fp = std::fopen(filepath, "wb");
    if (!fp)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return false;
    }
    _png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!_png)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return false;
    }
    _info = png_create_info_struct(_png);
    if (!_info)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return false;
    }
    png_init_io(_png, fp);
    png_set_compression_level(_png, Z_BEST_COMPRESSION);
    png_set_IHDR(_png, _info, _width, _height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(_png, _info);
    png_write_image(_png, _dest);
    png_write_png(_png, _info, PNG_TRANSFORM_IDENTITY, NULL);
    png_write_end(_png, _info);
    return true;
};

template <unsigned A>
PNGWrite<A>::~PNGWrite()
{
    destroy();
};

PNGWrite<0>::~PNGWrite()
{
    destroy();
};

template <unsigned A>
void PNGWrite<A>::destroy()
{
    png_destroy_write_struct(&_png, &_info);
};

void PNGWrite<0>::destroy()
{
    png_destroy_write_struct(&_png, &_info);
}

#endif
