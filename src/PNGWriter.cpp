#include "PNGWriter.h"
#include <iostream>
#include <stdio.h>
#include <zlib.h>
#include <png.h>


struct png_buffer
{
    unsigned char* buffer;
    size_t position;
};

void dummy_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
    size_t* size = reinterpret_cast<size_t*>(png_get_io_ptr(png_ptr));
    *size = *size + length;
}

void dummy_flash(png_structp png_ptr)
{
}

void png_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_buffer* png = reinterpret_cast<png_buffer*>(png_get_io_ptr(png_ptr));
    memcpy(png->buffer + png->position, data, length);
    png->position += length;
}

PNGWriter::~PNGWriter()
{
    if (_raw_buffer)
    {
        delete[] _raw_buffer;
        delete[] _image_rows;
    }
    if (_file_content)
    {
        delete[] _file_content;
    }
}

void PNGWriter::process(unsigned char* raw_buffer)
{
    std::cout << "store image src" << std::endl;
    _raw_buffer = raw_buffer;
    _image_rows = new unsigned char*[_height];
    size_t pixelSize = (_hasAlpha) ? 4 : 3;
    for (size_t i = 0; i < _height; ++i)
    {
        _image_rows[i] = raw_buffer + i * _width * pixelSize;
    }
    process(_image_rows);
}

void PNGWriter::process(unsigned char** image_rows)
{
    _file_size = 1 << 31;
    int filter;
    for (size_t i = 0; i < 5; ++i)
    {
        png_structp png = 0;
        png_infop info = 0;
        size_t file_size = 0;
        png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        if (!png)
        {
            return;
        }
        info = png_create_info_struct(png);
        if (!info)
        {
            return;
        }
        png_set_write_fn(png, reinterpret_cast<void*>(&file_size), dummy_write, dummy_flash);
        png_set_compression_level(png, Z_BEST_COMPRESSION);
        png_set_compression_mem_level(png, MAX_MEM_LEVEL);
        png_set_compression_window_bits(png, 15);
        png_set_filter(png, PNG_FILTER_TYPE_BASE, i);
        if (_hasAlpha)
        {
            png_set_IHDR(png, info, _width, _height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        }
        else
        {
            png_set_IHDR(png, info, _width, _height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        }
        png_write_info(png, info);
        png_write_image(png, image_rows);
        png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
        png_write_end(png, info);
        png_destroy_write_struct(&png, &info);
        std::cout << "filter type = " << i << " file size = " << file_size << std::endl;
        if (file_size < _file_size)
        {
            _file_size = file_size;
            filter = i; 
        }
    }
    if (_file_size != 1 << 31)
    {
        _valid = true;
        png_structp png = 0;
        png_infop info = 0;
        png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        if (!png)
        {
            return;
        }
        info = png_create_info_struct(png);
        if (!info)
        {
            return;
        }
        png_buffer buffer;
        _file_content = new unsigned char[_file_size];
        buffer.buffer = _file_content;
        buffer.position = 0;
        png_set_write_fn(png, reinterpret_cast<void*>(&buffer), png_write, dummy_flash);
        png_set_compression_level(png, Z_BEST_COMPRESSION);
        png_set_compression_mem_level(png, MAX_MEM_LEVEL);
        png_set_compression_window_bits(png, 15);
        png_set_filter(png, PNG_FILTER_TYPE_BASE, filter);
        if (_hasAlpha)
        {
            png_set_IHDR(png, info, _width, _height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        }
        else
        {
            png_set_IHDR(png, info, _width, _height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        }
        png_write_info(png, info);
        png_write_image(png, image_rows);
        png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
        png_write_end(png, info);
        png_destroy_write_struct(&png, &info);
    }
}

void PNGWriter::write(const char* filepath)
{
    if (_valid)
    {
        FILE * fp = fopen(filepath, "wb");
        fwrite(_file_content , 1 , _file_size , fp);
        fclose(fp);
    }
}
