#ifndef PALETTE_OPTIMIZER_H
#define PALETTE_OPTIMIZER_H

#include <iostream>
#include <map>
#include "boost/tr1/unordered_map.hpp"
#include "boost/tr1/unordered_set.hpp"

class PaletteOptimizer
{
public:
    explicit PaletteOptimizer(size_t width, size_t height);
    ~PaletteOptimizer();

    bool process32bit(unsigned char* src);
    bool process24bit(unsigned char* src);
    void process8bit(unsigned char* src , png_color* palette, unsigned char* trans);

    unsigned char* delegate_rawimage()
    {
        unsigned char* result = _dest;
        _dest = 0;
        return result;
    };
    png_color* delegate_palette()
    {
        png_color* result = _palette;
        _palette = 0;
        return result;
    };
    unsigned char* delegate_trans()
    {
        unsigned char* result = _trans;
        _trans = 0;
        return result;
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
    unsigned char* _dest;
    size_t _size;
    size_t _pixelsize;
    png_color* _palette;
    size_t _palette_size;
    unsigned char* _trans;
    size_t _trans_size;
    void destroy();
};

PaletteOptimizer::PaletteOptimizer(size_t width, size_t height)
    : _dest(0), _palette(0), _palette_size(0), _trans(0), _trans_size(0)
{
    _size = width * height;
};

bool PaletteOptimizer::process32bit(unsigned char* src)
{
}

bool PaletteOptimizer::process24bit(unsigned char* src)
{
}

void PaletteOptimizer::process8bit(unsigned char* src , png_color* palette, unsigned char* trans)
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
    _dest = new unsigned char[_size];
    _palette_size = optimized_palette.size();
    _palette = new png_color[_palette_size];
    _trans = new unsigned char[_trans_size];
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

PaletteOptimizer::~PaletteOptimizer()
{
    destroy();
}

void PaletteOptimizer::destroy()
{
    if (_dest)
    {
        delete[] _dest;
    }
    if (_palette)
    {
        delete[] _palette;
    }
    if (_trans)
    {
        delete[] _trans;
    }
};

#endif
