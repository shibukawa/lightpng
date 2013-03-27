#ifndef PNGWRITER_H
#define PNGWRITER_H

#include <png.h>
#include "LPType.h"
#include "Image.h"

class Buffer;

class PNGWriter
{
public:
    PNGWriter(Image& image, bool has_alpha, size_t optimize, bool verbose)
        : width_(0), height_(0), file_size_(0),
          has_alpha_(has_alpha), index_(false), optimize_(optimize), verbose_(verbose), valid_(0)
    {
        width_ = image.width();
        height_ = image.height();
    }
    ~PNGWriter();
    void process(buffer_t raw_buffer);
    void process(buffer_t raw_buffer, palette_t palette, trans_t trans, bool palette_optimize = false);
    void process(buffer_t raw_buffer, bool shrinkChannel);
    void write(const char* filepath);
    void compress(size_t parameter_index, Buffer* buffer);

private:
    buffer_t raw_buffer_;
    rows_t image_rows_;
    buffer_t file_content_;
    palette_t palette_;
    trans_t trans_;
    size_t width_;
    size_t height_;
    size_t file_size_;
    size_t palette_size_;
    size_t trans_size_;
    bool has_alpha_;
    bool index_;
    size_t optimize_;
    bool verbose_;
    bool valid_;

    void process_();
    bool can_convert_index_color_(buffer_t raw_buffer);
    int optimizeWithOptions_(size_t start_paramter, size_t end_parameter, bool show_result);
};

#endif
