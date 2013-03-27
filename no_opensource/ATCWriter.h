#ifndef ATCWRITER_H
#define ATCWRITER_H

#include "LPType.h"
#include <TextureConverter.h>

class ATCWriter
{
public:
    explicit ATCWriter(size_t width, size_t height)
        : texture_(0), width_(width), height_(height), size_(0) {}
    virtual ~ATCWriter();

    void process(buffer_t src, bool hasAlpha);
    void write(const char* filepath);
    void writeWithHeader(const char* filepath);
    void writeToPNG(const char* filepath);

private:
    TQonvertImage* texture_;
    size_t width_, height_, size_;

    void destroy();
};

#endif
