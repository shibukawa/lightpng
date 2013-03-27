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
    rows_t src_;
    rows_t dest_;
    buffer_t rawsrc_;
    buffer_t rawdest_;
    size_t width_, height_;
    unsigned R_, G_, B_;

    inline void get(rows_t& img, size_t x, size_t y, int& r, int& g, int& b, int& a)
    {
        clamp(x, 0, width_ - 1);
        clamp(y, 0, height_ - 1);

        r = static_cast<int>(img[y][4 * x]);
        g = static_cast<int>(img[y][4 * x + 1]);
        b = static_cast<int>(img[y][4 * x + 2]);
        a = static_cast<int>(img[y][4 * x + 3]);
    };

    inline void set(rows_t& img, size_t x, size_t y, int r, int g, int b, int a)
    {
        clamp(x, 0, width_ - 1);
        clamp(y, 0, height_ - 1);
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
        : width_(width), height_(height), R_(R), G_(G), B_(B)
    {
        rawsrc_.reset(new unsigned char[4 * width * height]);
        rawdest_.reset(new unsigned char[4 * width * height]);
        src_.reset(new unsigned char*[height]);
        dest_.reset(new unsigned char*[height]);
        for (size_t i = 0; i < height_; ++i)
        {
            src_[i] = rawsrc_.get() + (i * width * 4);
            dest_[i] = rawdest_.get() + (i * width * 4);
        }
    };

    void process(rows_t src, bool preview = true)
    {
        Image::copy_4_to_4(width_, height_, src, src_);
        const int MaxR = (1 << R_) - 1;
        const int MaxG = (1 << G_) - 1;
        const int MaxB = (1 << B_) - 1;
        const int MaxA = (1 << A) - 1;
        const int OriginalMax = (1 << 8) - 1;
  
        float f_r, f_g, f_b, f_a;
        if (preview)
        {
            f_r = 255.0 / (256 - (1 << (8 - R_)));
            f_g = 255.0 / (256 - (1 << (8 - G_)));
            f_b = 255.0 / (256 - (1 << (8 - B_)));
            f_a = 255.0 / (256 - (1 << (8 - A)));
        }
        else
        {
            f_r = 1;
            f_g = 1;
            f_b = 1;
            f_a = 1;
        }
        for (size_t y = 0; y < height_; ++y)
        {
            for (size_t x = 0; x < width_; ++x)
            {
                int old_r, old_g, old_b, old_a;
                get(src_, x, y, old_r, old_g, old_b, old_a);
            
                unsigned char new_r = (static_cast<int>(old_r) * MaxR / OriginalMax) << (8 - R_);
                unsigned char new_g = (static_cast<int>(old_g) * MaxG / OriginalMax) << (8 - G_);
                unsigned char new_b = (static_cast<int>(old_b) * MaxB / OriginalMax) << (8 - B_);
                unsigned char new_a = (static_cast<int>(old_a) * MaxA / OriginalMax) << (8 - A);

                set(dest_, x, y, static_cast<int>(f_r * new_r), static_cast<int>(f_g * new_g), static_cast<int>(f_b * new_b), static_cast<int>(f_a * new_a));

                int dr = std::abs(old_r - new_r);
                int dg = std::abs(old_g - new_g);
                int db = std::abs(old_b - new_b);
                int da = std::abs(old_a - new_a);

                int r, g, b, a;
                get(src_, x + 1, y, r, g, b, a);
                r += dr * 7 / 16;
                g += dg * 7 / 16;
                b += db * 7 / 16;
                a += da * 7 / 16;
                set(src_, x + 1, y, r, g, b, a);

                get(src_, x - 1, y + 1, r, g, b, a);
                r += dr * 3 / 16;
                g += dg * 3 / 16;
                b += db * 3 / 16;
                a += da * 3 / 16;
                set(src_, x - 1, y + 1, r, g, b, a);

                get(src_, x, y + 1, r, g, b, a);
                r += dr * 5 / 16;
                g += dg * 5 / 16;
                b += db * 5 / 16;
                a += da * 5 / 16;
                set(src_, x, y + 1, r, g, b, a);

                get(src_, x + 1, y + 1, r, g, b, a);
                r += dr * 1 / 16;
                g += dg * 1 / 16;
                b += db * 1 / 16;
                a += da * 1 / 16;
                set(src_, x + 1, y + 1, r, g, b, a);
            }
        }
    };

    buffer_t buffer() {
        return rawdest_;
    };
};

template<>
class ColorReducer<0>
{
private:
    rows_t src_;
    rows_t dest_;
    buffer_t rawsrc_;
    buffer_t rawdest_;
    size_t width_, height_;
    unsigned R_, G_, B_;

    inline void get(rows_t& img, size_t x, size_t y, int& r, int& g, int& b)
    {
        clamp(x, 0, width_ - 1);
        clamp(y, 0, height_ - 1);

        r = static_cast<int>(img[y][3 * x]);
        g = static_cast<int>(img[y][3 * x + 1]);
        b = static_cast<int>(img[y][3 * x + 2]);
    };

    inline void set(rows_t& img, size_t x, size_t y, int r, int g, int b)
    {
        clamp(x, 0, width_ - 1);
        clamp(y, 0, height_ - 1);
        clamp(r, 0, 255);
        clamp(g, 0, 255);
        clamp(b, 0, 255);

        img[y][3 * x]     = static_cast<unsigned char>(r);
        img[y][3 * x + 1] = static_cast<unsigned char>(g);
        img[y][3 * x + 2] = static_cast<unsigned char>(b);
    };
public:
    explicit ColorReducer(size_t width, size_t height, unsigned R, unsigned G, unsigned B)
        : width_(width), height_(height), R_(R), G_(G), B_(B)
    {
        rawsrc_.reset(new unsigned char[3 * width * height]);
        rawdest_.reset(new unsigned char[3 * width * height]);
        src_.reset(new unsigned char*[height]);
        dest_.reset(new unsigned char*[height]);
        for (size_t i = 0; i < height; ++i)
        {
            src_[i] = rawsrc_.get() + (i * width * 3);
            dest_[i] = rawdest_.get() + (i * width * 3);
        }
    };

    void process(rows_t src, bool hasAlphaChannel, bool preview = true)
    {
        if (hasAlphaChannel)
        {
            Image::copy_4_to_3(width_, height_, src, src_);
        }
        else
        {
            Image::copy_3_to_3(width_, height_, src, src_);
        }
        const int MaxR = (1 << R_) - 1;
        const int MaxG = (1 << G_) - 1;
        const int MaxB = (1 << B_) - 1;
        const int OriginalMax = (1 << 8) - 1;

        float f_r, f_g, f_b;
        if (preview)
        {
            f_r = 255.0 / (256 - (1 << (8 - R_)));
            f_g = 255.0 / (256 - (1 << (8 - G_)));
            f_b = 255.0 / (256 - (1 << (8 - B_)));
        }
        else
        {
            f_r = 1;
            f_g = 1;
            f_b = 1;
        }

        for (size_t y = 0; y < height_; ++y)
        {
            for (size_t x = 0; x < width_; ++x)
            {
                int old_r, old_g, old_b;
                get(src_, x, y, old_r, old_g, old_b);
                
                unsigned char new_r = (static_cast<int>(old_r) * MaxR / OriginalMax) << (8 - R_);
                unsigned char new_g = (static_cast<int>(old_g) * MaxG / OriginalMax) << (8 - G_);
                unsigned char new_b = (static_cast<int>(old_b) * MaxB / OriginalMax) << (8 - B_);

                set(dest_, x, y, static_cast<int>(f_r * new_r), static_cast<int>(f_g * new_g), static_cast<int>(f_b * new_b));

                int dr = std::abs(old_r - new_r);
                int dg = std::abs(old_g - new_g);
                int db = std::abs(old_b - new_b);

                int r, g, b;
                get(src_, x + 1, y, r, g, b);
                r += dr * 7 / 16;
                g += dg * 7 / 16;
                b += db * 7 / 16;
                set(src_, x + 1, y, r, g, b);

                get(src_, x - 1, y + 1, r, g, b);
                r += dr * 3 / 16;
                g += dg * 3 / 16;
                b += db * 3 / 16;
                set(src_, x - 1, y + 1, r, g, b);

                get(src_, x, y + 1, r, g, b);
                r += dr * 5 / 16;
                g += dg * 5 / 16;
                b += db * 5 / 16;
                set(src_, x, y + 1, r, g, b);

                get(src_, x + 1, y + 1, r, g, b);
                r += dr * 1 / 16;
                g += dg * 1 / 16;
                b += db * 1 / 16;
                set(src_, x + 1, y + 1, r, g, b);
            }
        }
    };
    buffer_t buffer() {
        return rawdest_;
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
