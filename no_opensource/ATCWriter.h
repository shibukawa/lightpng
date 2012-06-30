#ifndef ATCWRITER_H
#define ATCWRITER_H

#include <TextureConverter.h>

class ATCWriter
{
public:
    explicit ATCWriter(size_t width, size_t height)
        : _texture(0), _width(width), _height(height) {}
    virtual ~ATCWriter();

    void process(unsigned char* src, bool hasAlpha);
    void write(const char* filepath);
    void writeWithHeader(const char* filepath);
    void writeToPNG(const char* filepath);

private:
    TQonvertImage* _texture;
    size_t _width, _height;

    void destroy();
};

#endif
