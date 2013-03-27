#ifndef MEDIAN_CUT_16BIT_QUANTIZER_H
#define MEDIAN_CUT_16BIT_QUANTIZER_H

#include <iostream>
#include <boost/scoped_array.hpp>
#include "Quantizer.h"
#include "LPType.h"
#include "Image.h"
#include "PNGWriter.h"

const int NUM_DIMENSIONS = 4;

struct Point
{
    unsigned char x[NUM_DIMENSIONS];
};

class Block
{
    Point minCorner_, maxCorner_;
    Point* points_;
    int pointsLength_;
    int colorIndex_;
    size_t longestSideIndex_;
    size_t longestSideLength_;
public:
    Block(Point* points, int pointsLength);
    Point* getPoints() { return points_; }
    int numPoints() const { return pointsLength_; }
    inline int longestSideIndex() const { return longestSideIndex_; };
    inline int longestSideLength() const { return longestSideLength_ ;};
    int colorIndex() const { return colorIndex_; };
    int calcLongestSide(int R, int G, int B, int A);
    bool operator<(const Block& rhs) const;
    void shrink();
    Point* minCorner() { return &minCorner_; }
    Point* maxCorner() { return &maxCorner_; }
    void setColorIndex(int R, int G, int B, int A, size_t colorIndex, boost::scoped_array<short>& colorMap);
    void calcAverageColor(unsigned char&, unsigned char&, unsigned char&, unsigned char&);
private:
    template <typename T>
    static T min(const T a, const T b)
    {
        if (a < b)
            return a;
        else
            return b;
    }

    template <typename T>
    static T max(const T a, const T b)
    {
        if (a > b)
            return a;
        else
            return b;
    }
};


class MedianCut16bitQuantizer : public Quantizer
{
public:
    explicit MedianCut16bitQuantizer(size_t width, size_t height) : Quantizer(width, height) {}

    void quantize(size_t R, size_t G, size_t B, size_t A);
    void fixPalette(size_t R, size_t G, size_t B, size_t A);
private:
    bool preview_;
    std::vector<boost::shared_ptr<Block> > blocks_;

    short searchNearestColor(int r, int g, int b, int a, bool skipAlpha = false);

    inline void get(buffer_t& img, size_t x, size_t y, int& r, int& g, int& b, int& a)
    {
        clamp(x, 0, width_ - 1);
        clamp(y, 0, height_ - 1);

        size_t offset = (y * width_ + x) * 4;
        r = static_cast<int>(img[offset]);
        g = static_cast<int>(img[offset + 1]);
        b = static_cast<int>(img[offset + 2]);
        a = static_cast<int>(img[offset + 3]);
    };

    inline void set(buffer_t& img, size_t x, size_t y, int index)
    {
        clamp(x, 0, width_ - 1);
        clamp(y, 0, height_ - 1);
        clamp(index, 0, 255);

        img[y * width_ + x] = static_cast<unsigned char>(index);
    };

    inline void set(buffer_t& img, size_t x, size_t y, int r, int g, int b, int a)
    {
        clamp(x, 0, width_ - 1);
        clamp(y, 0, height_ - 1);
        clamp(r, 0, 255);
        clamp(g, 0, 255);
        clamp(b, 0, 255);
        clamp(a, 0, 255);

        size_t offset = (y * width_ + x) * 4;
        img[offset]     = static_cast<unsigned char>(r);
        img[offset + 1] = static_cast<unsigned char>(g);
        img[offset + 2] = static_cast<unsigned char>(b);
        img[offset + 3] = static_cast<unsigned char>(a);
    };
};


void median_cut_16bit_quantize(Image& image, PNGWriter& writer, bool hasAlphaChannel, bool hasAlpha, bool preview);


#endif
