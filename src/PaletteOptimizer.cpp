#include <iostream>
#include <boost/tr1/unordered_map.hpp>
#include <boost/tr1/unordered_set.hpp>
#include "PaletteOptimizer.h"

PaletteOptimizer::PaletteOptimizer(size_t width, size_t height)
    : _palette_size(0), _trans_size(0)
{
    _size = width * height;
};

bool PaletteOptimizer::process32bit(buffer_t src)
{
}

bool PaletteOptimizer::process24bit(buffer_t src)
{
}

void PaletteOptimizer::process8bit(buffer_t src , palette_t palette, trans_t trans)
{
    boost::unordered_map<unsigned char, unsigned char> palette_convert;
    boost::unordered_map<unsigned int, unsigned char> optimized_palette;
    boost::unordered_set<unsigned char> used_palette;

    for (size_t i = 0; i < _size; i++)
    {
        used_palette.insert(src[i]);
    }
    boost::unordered_set<unsigned char>::iterator it;

    for (size_t i = 0; i < 256; i++)
    {
        unsigned char c = static_cast<unsigned char>(i);
        if (used_palette.find(c) == used_palette.end())
        {
            continue;
        }
        unsigned int color = (palette[i].red << 24) + (palette[i].green << 16) + (palette[i].blue << 8) + trans[i];
        boost::unordered_map<unsigned int, unsigned char>::iterator existing = optimized_palette.find(color);
        if (existing != optimized_palette.end())
        {
            palette_convert[i] = palette_convert[existing->second];
        }
        else
        {
            palette_convert[i] = optimized_palette.size();
            optimized_palette[color] = c;
            if (trans[i] != 255)
            {
                _trans_size++;
            }
        }
    }
    _dest.reset(new unsigned char[_size]);
    _palette_size = optimized_palette.size();
    _palette.reset(new png_color[_palette_size]);
    _trans.reset(new unsigned char[_trans_size]);
    boost::unordered_map<unsigned int, unsigned char>::iterator iter;
    for (iter = optimized_palette.begin(); iter != optimized_palette.end(); iter++)
    {
        size_t offset = palette_convert[iter->second];
        _palette[offset] = palette[iter->second];
        if (offset < _trans_size)
        {
            _trans[offset] = trans[iter->second];
        }
    }
    for (size_t i = 0; i < _size; i++)
    {
        _dest[i] = palette_convert[src[i]];
    }
}

