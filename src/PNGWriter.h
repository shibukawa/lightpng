#ifndef PNGWRITER_H
#define PNGWRITER_H


#include <iostream>
#include <png.h>
#include "Image.h"


class PNGWriter
{
public:
    PNGWriter(Image* image, bool hasAlphaChannel, bool verbose)
        : _raw_buffer(0), _image_rows(0), _file_content(0),
          _width(0), _height(0), _file_size(0),
          _hasAlpha(hasAlphaChannel), _verbose(verbose), _valid(0)
    {
        _width = image->width();
        _height = image->height();
    }
    ~PNGWriter();
    void process(unsigned char* raw_buffer);
    void process(unsigned char** image_rows);
    void write(const char* filepath);
    int compress(size_t parameter_index, bool in_memory_test = true);

private:
    unsigned char* _raw_buffer;
    unsigned char** _image_rows;
    unsigned char* _file_content;
    size_t _width;
    size_t _height;
    size_t _file_size;
    bool _hasAlpha;
    bool _verbose;
    bool _valid;
};

#endif
