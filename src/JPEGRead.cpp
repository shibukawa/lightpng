#include <stdio.h>
#include <iostream>
#include "JPEGRead.h"

JPEGRead::JPEGRead(const char* filepath) : Read()
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
    
    JSAMPARRAY raw_data = static_cast<JSAMPARRAY>(new JSAMPROW[_height]);
    for (int i = 0; i < _height; i++) {
        raw_data[i] = const_cast<JSAMPROW>(new JSAMPLE[_width * 3]);
    }

    while(_jpeginfo.output_scanline < _jpeginfo.output_height)
    {
        jpeg_read_scanlines(&_jpeginfo,
            raw_data + _jpeginfo.output_scanline,
            _jpeginfo.output_height - _jpeginfo.output_scanline
        );
    }
    _data = static_cast<unsigned char**>(raw_data);

    jpeg_finish_decompress(&_jpeginfo);
    fclose(fp);
    _valid = true;
}

JPEGRead::~JPEGRead()
{
    destroy();
}

void JPEGRead::destroy()
{
    if (_data)
    {
        for (int i = 0; i < _height; i++ )
        {
            delete[] _data[i];
        }
        delete[] _data;
    }
    jpeg_destroy_decompress(&_jpeginfo);
}
