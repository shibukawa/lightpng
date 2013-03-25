#ifndef PNGWRITER_H
#define PNGWRITER_H

#include <png.h>
#include "LPType.h"
#include "Image.h"

class Buffer;

class PNGWriter
{
public:
    PNGWriter(Image& image, bool has_alpha, bool optimize, bool verbose)
        : _width(0), _height(0), _file_size(0),
          _has_alpha(has_alpha), _index(false), _optimize(optimize), _verbose(verbose), _valid(0)
    {
        _width = image.width();
        _height = image.height();
    }
    ~PNGWriter();
    void process(buffer_t raw_buffer);
    void process(buffer_t raw_buffer, palette_t palette, trans_t trans, bool palette_optimize = false);
    void process(buffer_t raw_buffer, bool shrinkChannel);
    void write(const char* filepath);
    void compress(size_t parameter_index, Buffer* buffer);

private:
    buffer_t _raw_buffer;
    rows_t _image_rows;
    buffer_t _file_content;
    palette_t _palette;
    trans_t _trans;
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

    void _process();
    bool _can_convert_index_color(buffer_t raw_buffer);
};

#endif
