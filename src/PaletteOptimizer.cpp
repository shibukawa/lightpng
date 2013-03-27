#include <iostream>
#include <boost/tr1/unordered_map.hpp>
#include <boost/tr1/unordered_set.hpp>
#include "PaletteOptimizer.h"

PaletteOptimizer::PaletteOptimizer(size_t width, size_t height)
    : palette_size_(0), trans_size_(0)
{
    size_ = width * height;
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
    boost::unordered_set<unsigned char> usedpalette_;

    for (size_t i = 0; i < size_; i++)
    {
        usedpalette_.insert(src[i]);
    }
    boost::unordered_set<unsigned char>::iterator it;

    for (size_t i = 0; i < 256; i++)
    {
        unsigned char c = static_cast<unsigned char>(i);
        if (usedpalette_.find(c) == usedpalette_.end())
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
                trans_size_++;
            }
        }
    }
    dest_.reset(new unsigned char[size_]);
    palette_size_ = optimized_palette.size();
    palette_.reset(new png_color[palette_size_]);
    trans_.reset(new unsigned char[trans_size_]);
    boost::unordered_map<unsigned int, unsigned char>::iterator iter;
    for (iter = optimized_palette.begin(); iter != optimized_palette.end(); iter++)
    {
        size_t offset = palette_convert[iter->second];
        palette_[offset] = palette[iter->second];
        if (offset < trans_size_)
        {
            trans_[offset] = trans[iter->second];
        }
    }
    for (size_t i = 0; i < size_; i++)
    {
        dest_[i] = palette_convert[src[i]];
    }
}

