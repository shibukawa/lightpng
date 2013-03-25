#ifndef MEDIAN_CUT_32BIT_QUANTIZER_H
#define MEDIAN_CUT_32BIT_QUANTIZER_H

#include "Quantizer.h"
#include "LPType.h"
#include "Image.h"
#include "PNGWriter.h"

static inline unsigned char CLAMP(int value)
{
    return (value >= 0 ? (value <= 255 ? value : 255) : 0);
}

class MedianCut32bitQuantizer : public Quantizer
{
public:
    MedianCut32bitQuantizer(size_t width, size_t height) : Quantizer(width, height) {}
private:
    void _process();
};

void median_cut_32bit_quantize(Image& image, PNGWriter& writer, bool hasAlphaChannel);

#endif
