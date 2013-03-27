#ifndef BIT_CHANGER_H
#define BIT_CHANGER_H

#include <math.h>
#include "LPType.h"
#include "Image.h"

class BitChanger : public Image
{
public:
    explicit BitChanger(size_t width, size_t height, bool useAlpha, buffer_t src)
        : Image()
    {
        size_t size = pow(2, ceil(log2(fmax(width, height))));
        width_ = size;
        height_ = size;

        alloc(4);

        int bytePerPixel = (useAlpha) ? 4 : 3;

        for (size_t y = 0; y < size; y++)
        {
            for (size_t x = 0; x < size; x++)
            {
                if (y < height && x < width)
                {
                    size_t src_offset = y * width + x;
                    size_t dest_offset = y * size + x;
                    data_[dest_offset * 4]     = src[src_offset * bytePerPixel];
                    data_[dest_offset * 4 + 1] = src[src_offset * bytePerPixel + 1];       
                    data_[dest_offset * 4 + 2] = src[src_offset * bytePerPixel + 2];
                    if (useAlpha)
                    {
                        data_[dest_offset * 4 + 3] = src[src_offset * bytePerPixel + 3];
                    }
                    else
                    {
                        data_[dest_offset * 4 + 3] = 255;
                    }
                }
                else
                {
                    size_t dest_offset = y * size + x;
                    data_[dest_offset * 4] = 0;
                    data_[dest_offset * 4 + 1] = 0;
                    data_[dest_offset * 4 + 2] = 0;
                    data_[dest_offset * 4 + 3] = 0;
                }
            }
        }
        valid_ = true;
    }
};

#endif
