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
        return _dest;
    }
    palette_t palette()
    {
        return _palette;
    }
    trans_t trans()
    {
        return _trans;
    }
    size_t palette_size()
    {
        return _palette_size;
    }
    size_t trans_size()
    {
        return _trans_size;
    }

private:
    buffer_t _dest;
    size_t _size;
    size_t _pixelsize;
    palette_t _palette;
    size_t _palette_size;
    trans_t _trans;
    size_t _trans_size;
};

#endif
