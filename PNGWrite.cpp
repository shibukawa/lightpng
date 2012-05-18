#include <png.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "PNGRead.h"

static void _read(png_structp pngp, png_bytep data, png_size_t length)
{
    std::istream* infile = static_cast<std::istream*>(png_get_io_ptr(pngp));
    infile->read(reinterpret_cast<char*>(data), length);
}

PNGWrite::PNGWrite(const char* filepath, size_t width, size_t height, bool has_alpha)
    : _png(0), _info(0), _end(0), _data(0), _width(width), _height(height),
      _has_alpha(has_alpha)
{
    std::cout << filepath << std::endl;

    FILE* fp = fopen(filepath, "rb");
    if (!fp)
    {
        std::cout << "Input file not found: " << filepath << std::endl;
        return;
    }

    _png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if(!_png)
    {
        return;
    }
    _info = png_create_info_struct(_png);
    if(!_info)
    {
        return;
    }
    _end = png_create_info_struct(_png);
    if(!_end)
    {
        return;
    }
    png_init_io(_png, fp);
    png_read_png(_png, _info, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16, NULL);
    _width = png_get_image_width(_png, _info);
    _height = png_get_image_height(_png, _info);
    _channels = png_get_channels(_png, _info);
    std::cout << "width = " << _width << "\n";
    std::cout << "height = " << _height << "\n";
    std::cout << "channels = " << _channels << "\n";
    _data = png_get_rows(_png, _info);

    _valid = true;
}

PNGRead::~PNGRead()
{
    destroy();
}

void PNGRead::destroy()
{
    png_destroy_read_struct(&_png, &_info, &_end);
}
