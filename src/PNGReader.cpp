#include <png.h>
#include <stdio.h>
#include <iostream>
#include "PNGReader.h"


PNGReader::PNGReader(const char* filepath) : Image(), _png(0), _info(0), _channels(0)
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
    png_read_info(_png, _info);
    if (png_get_color_type(_png, _info) == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_palette_to_rgb(_png);
        if (png_get_valid(_png, _info, PNG_INFO_tRNS))
        {
            png_set_tRNS_to_alpha(_png);
            _channels = 4;
        }
        else
        {
            _channels = 3;
        }
    }
    else if (png_get_color_type(_png, _info) == PNG_COLOR_TYPE_GRAY)
    {
        std::cout << "gray scale color" << std::endl;
        png_set_gray_to_rgb(_png);
        if (png_get_valid(_png, _info, PNG_INFO_tRNS))
        {
            std::cout << "it has transparent information" << std::endl;
            png_set_tRNS_to_alpha(_png);
            _channels = 4;
        }
        else
        {
            _channels = 3;
        }
    }
    else
    {
        int bitdepth = png_get_bit_depth(_png, _info);
        if (bitdepth == 16)
        {
            png_set_strip_16(_png);
        }
        else if ((bitdepth < 8) || (png_get_valid(_png, _info, PNG_INFO_tRNS)))
        {
            png_set_expand(_png);
        }
        _channels = png_get_channels(_png, _info);
    }

    _width = png_get_image_width(_png, _info);
    _height = png_get_image_height(_png, _info);

    alloc(_channels);

    png_read_image(_png, image().get());
    png_read_end(_png, NULL);

    checkHasAlpha();

    _valid = true;
}

void PNGReader::checkHasAlpha() {
    _hasAlpha = false;
    if (_channels == 3)
    {
        return;
    }
    for (size_t y = 0; y < _height; y++)
    {
        unsigned char* row = _rows[y];
        for (size_t x = 0; x < _width; x++)
        {
            if (row[x * 4 + 3] != 255)
            {
                _hasAlpha = true;
                return;
            }
        }
    }
}

PNGReader::~PNGReader()
{
    destroy();
}

void PNGReader::destroy()
{
    png_destroy_read_struct(&_png, &_info, NULL);
}
