#include <stdio.h>
#include <iostream>
#include "JPEGReader.h"


JPEGReader::JPEGReader(const char* filepath) : Image()
{
    jpeg_create_decompress(&_jpeginfo);
    _jpeginfo.err = jpeg_std_error(&_jpegerr);

    FILE* fp = fopen(filepath, "rb");
    if (!fp)
    {
        std::cout << "Input file not found: " << filepath << std::endl;
        return;
    }
    jpeg_stdio_src(&_jpeginfo, fp);
    jpeg_read_header(&_jpeginfo, TRUE);
    jpeg_start_decompress(&_jpeginfo);

    if (_jpeginfo.out_color_components != 3)
    {
        std::cout << "This program support only 24 bit image." << std::endl;
        fclose(fp);
        return;
    }
    _width = _jpeginfo.output_width;
    _height = _jpeginfo.output_height;

    alloc(3);

    while(_jpeginfo.output_scanline < _jpeginfo.output_height)
    {
        jpeg_read_scanlines(&_jpeginfo,
            raw_image() + _jpeginfo.output_scanline,
            _jpeginfo.output_height - _jpeginfo.output_scanline
        );
    }

    jpeg_finish_decompress(&_jpeginfo);
    fclose(fp);
    _valid = true;
}

JPEGReader::~JPEGReader()
{
    destroy();
}

void JPEGReader::destroy()
{
    jpeg_destroy_decompress(&_jpeginfo);
}
