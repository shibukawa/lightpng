#include <stdio.h>
#include <iostream>
#include "JPEGReader.h"


JPEGReader::JPEGReader(const char* filepath) : Image()
{
    jpeg_create_decompress(&jpeginfo_);
    jpeginfo_.err = jpeg_std_error(&jpegerr_);

    FILE* fp = fopen(filepath, "rb");
    if (!fp)
    {
        std::cout << "Input file not found: " << filepath << std::endl;
        return;
    }
    jpeg_stdio_src(&jpeginfo_, fp);
    jpeg_read_header(&jpeginfo_, TRUE);
    jpeg_start_decompress(&jpeginfo_);

    if (jpeginfo_.out_color_components != 3)
    {
        std::cout << "This program support only 24 bit image." << std::endl;
        fclose(fp);
        return;
    }
    width_ = jpeginfo_.output_width;
    height_ = jpeginfo_.output_height;

    alloc(3);

    while(jpeginfo_.output_scanline < jpeginfo_.output_height)
    {
        jpeg_read_scanlines(&jpeginfo_,
            image().get() + jpeginfo_.output_scanline,
            jpeginfo_.output_height - jpeginfo_.output_scanline
        );
    }

    jpeg_finish_decompress(&jpeginfo_);
    fclose(fp);
    valid_ = true;
}

JPEGReader::~JPEGReader()
{
    destroy();
}

void JPEGReader::destroy()
{
    jpeg_destroy_decompress(&jpeginfo_);
}
