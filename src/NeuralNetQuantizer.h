#ifndef NEURAL_NET_QUANTIZER_H
#define NEURAL_NET_QUANTIZER_H

#include "Quantizer.h"
#include "LPType.h"
#include "Image.h"
#include "PNGWriter.h"
extern "C" {
    #include "neuquant32.h"
}

static inline unsigned char CLAMP(int value)
{
    return (value >= 0 ? (value <= 255 ? value : 255) : 0);
}

class NeuralNetQuantizer : public Quantizer
{
public:
    NeuralNetQuantizer(size_t width, size_t height) : Quantizer(width, height) {}
private:
    void _process();
    void remap_floyd(unsigned char map[MAXNETSIZE][4], size_t* remap);
    void remap_simple(unsigned char map[MAXNETSIZE][4], size_t* remap);
};

void neural_net_quantize(Image& image, PNGWriter& writer, bool hasAlphaChannel);

#endif
