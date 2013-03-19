#ifndef PNGWRITER_H
#define PNGWRITER_H

#include <png.h>
#include "Image.h"

class Buffer;

class PNGWriter
{
public:
    PNGWriter(Image* image, bool has_alpha, bool optimize, bool verbose)
        : _raw_buffer(0), _image_rows(0), _file_content(0),
          _palette(0), _trans(0), _width(0), _height(0), _file_size(0),
          _has_alpha(has_alpha), _index(false), _optimize(optimize), _verbose(verbose), _valid(0)
    {
        _width = image->width();
        _height = image->height();
    }
    ~PNGWriter();
    void process(unsigned char* raw_buffer);
    void process(unsigned char* raw_buffer, png_color* palette, unsigned char* trans);
    void process(unsigned char** image_rows);
    void write(const char* filepath);
    void compress(size_t parameter_index, Buffer* buffer);

private:
    unsigned char* _raw_buffer;
    unsigned char** _image_rows;
    unsigned char* _file_content;
    png_color* _palette;
    unsigned char* _trans;
    size_t _width;
    size_t _height;
    size_t _file_size;
    size_t _palette_size;
    size_t _trans_size;
    bool _has_alpha;
    bool _index;
    bool _optimize;
    bool _verbose;
    bool _valid;
};

#endif
