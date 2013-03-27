#ifndef PALETTE_OPTIMIZER_H
#define PALETTE_OPTIMIZER_H

#include "LPType.h"

class PaletteOptimizer
{
public:
    explicit PaletteOptimizer(size_t width, size_t height);

    bool process32bit(buffer_t src);
    bool process24bit(buffer_t src);
    void process8bit(buffer_t src , palette_t palette, trans_t trans);

    buffer_t buffer()
    {
        return dest_;
    }
    palette_t palette()
    {
        return palette_;
    }
    trans_t trans()
    {
        return trans_;
    }
    size_t palette_size()
    {
        return palette_size_;
    }
    size_t trans_size()
    {
        return trans_size_;
    }

private:
    buffer_t dest_;
    size_t size_;
    size_t pixelsize_;
    palette_t palette_;
    size_t palette_size_;
    trans_t trans_;
    size_t trans_size_;
};

#endif
