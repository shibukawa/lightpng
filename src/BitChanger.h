#ifndef EXPAND_READ_H
#define EXPAND_READ_H

#include "Read.h"

class ExpandRead : public Read
{
public:
    explicit ExpandRead(size_t width, size_t height, unsigned char* src)
        : Read()
    {
        _width = width;
        _height = height;
        alloc(4);

        for (size_t y = 0; y < _height; y++)
        {
            for (size_t x = 0; x < _height; x++)
            {
                size_t offset = y * _width + x;
                _data[offset * 4]     = src[offset * 3];       
                _data[offset * 4 + 1] = src[offset * 3 + 1];       
                _data[offset * 4 + 2] = src[offset * 3 + 1];       
                _data[offset * 4 + 3] = 255;       
            }
        }
        _valid = true;
    }
};

#endif
