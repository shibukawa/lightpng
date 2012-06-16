#include <png.h>
#include <stdio.h>
#include <iostream>
#include "PNGRead.h"


PNGRead::PNGRead(const char* filepath) : Read(), _png(0), _info(0), _channels(0)
{
    FILE* fp = fopen(filepath, "rb");
    if (!fp)
    {
        std::cout << "Input file not found: " << filepath << std::endl;
        return;
    }

    _png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if(!_png)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return;
    }
    _info = png_create_info_struct(_png);
    if(!_info)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return;
    }
    _end = png_create_info_struct(_png);
    if(!_end)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return;
    }
    png_init_io(_png, fp);
    png_read_png(_png, _info, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16, NULL);
    _width = png_get_image_width(_png, _info);
    _height = png_get_image_height(_png, _info);
    _channels = png_get_channels(_png, _info);
    _data = static_cast<unsigned char**>(png_get_rows(_png, _info));

    _valid = true;
}

PNGRead::~PNGRead()
{
    destroy();
}

void PNGRead::destroy()
{
    png_destroy_read_struct(&_png, &_info, NULL);
}
