#ifndef COLOR_QUANTIZER_H
#define COLOR_QUANTIZER_H

#include <iostream>
#include <cstring>
#include "Image.h"
#include "PNGWriter.h"
extern "C" {
    #include "neuquant32.h"
}

static inline unsigned char CLAMP(int value)
{
    return (value >= 0 ? (value <= 255 ? value : 255) : 0);
}

class ColorQuantizer
{
public:
    explicit ColorQuantizer(size_t width, size_t height);
    virtual ~ColorQuantizer();

    operator void*() const { return _valid ? const_cast<ColorQuantizer*>(this) : 0; }
    bool operator!() const { return !_valid; }

    void process(unsigned char** src);
    unsigned char* delegate_rawimage()
    {
        unsigned char* result = _rawdest;
        _rawdest = 0;
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
    size_t trans_size()
    {
        return _transnum;
    }

private:
    unsigned char** _src;
    unsigned char** _dest;
    unsigned char* _rawsrc;
    unsigned char* _rawdest;
    size_t _width, _height;
    size_t _transnum;
    bool _valid;
    png_color* _palette;
    unsigned char* _trans;

    void destroy();
    void remap_floyd(unsigned char map[MAXNETSIZE][4], size_t* remap);
};

ColorQuantizer::ColorQuantizer(size_t width, size_t height)
    : _src(0), _dest(0), _rawsrc(0), _rawdest(0),
      _width(width), _height(height), _transnum(0), _valid(false), _palette(0), _trans(0)
{
    _rawsrc = new unsigned char[4 * width * height];
    _rawdest = new unsigned char[width * height];
    _src = new unsigned char*[height];
    _dest = new unsigned char*[height];
    for (size_t i = 0; i < _height; ++i)
    {
        _src[i] = _rawsrc + (i * width * 4);
        _dest[i] = _rawdest + (i * width);
    }
    _valid = true;
    _palette = new png_color[256];
    _trans = new unsigned char[256];
};

void ColorQuantizer::process(unsigned char** src)
{
    for (size_t i = 0; i < _height; ++i)
    {
        memcpy(_src[i], src[i], _width * 3);
    }
    size_t bot_idx, top_idx;  // for remapping of indices
    size_t x;
    size_t remap[MAXNETSIZE];

    size_t y;
    unsigned char map[MAXNETSIZE][4];
    int newcolors = 256;

    // Start neuquant
    initnet(_rawsrc, _height * _width * 4, newcolors, 1.0);
    int verbose = 0;
    learn(1, verbose);
    inxbuild();
    getcolormap((unsigned char*)map);

    // Remap indexes so all tRNS chunks are together
    //PNGNQ_MESSAGE("  Remapping colormap to eliminate opaque tRNS - chunk entries...\n");
    for (top_idx = newcolors - 1, x = 0;  x < newcolors;  ++x)
    {
        if (map[x][3] ==  255)  // maxval
        {
            remap[x] = top_idx--;
        }
        else
        {
            remap[x] = _transnum++;
        }
    }

    // PNGNQ_MESSAGE( "%d entr%s left\n", bot_idx,(bot_idx ==  1)? "y" : "ies");

    // GRR TO DO:  if bot_idx ==  0, check whether all RGB samples are gray
    // and if so, whether grayscale sample_depth would be same
    // = > skip following palette section and go grayscale

    // Remap and make palette entries
    for (x = 0; x < newcolors; ++x)
    {
        _palette[remap[x]].red   = map[x][0];
        _palette[remap[x]].green = map[x][1];
        _palette[remap[x]].blue  = map[x][2];
        _trans[remap[x]]         = map[x][3];
    }
    remap_floyd(map, remap);
}

void ColorQuantizer::remap_floyd(unsigned char map[MAXNETSIZE][4], size_t* remap)
{
    for (size_t y = 0; y < _height; ++y)
    {
        int offset, nextoffset;

        int rerr = 0;
        int berr = 0;
        int gerr = 0;
        int aerr = 0;

        offset = y * _width * 4;
        nextoffset = offset;
        if (y + 1 < _height)
        {
            nextoffset  += _width * 4;
        }
        int increment = 4;

        for (size_t i = 0; i < _width; i++ , offset  += increment, nextoffset  += increment)
        {
            int idx;
            unsigned int floyderr = rerr * rerr + gerr * gerr + berr * berr + aerr * aerr;

            idx = inxsearch(CLAMP(_rawsrc[offset + 3] - aerr),
                            CLAMP(_rawsrc[offset + 2] - berr),
                            CLAMP(_rawsrc[offset + 1] - gerr),
                            CLAMP(_rawsrc[offset]     - rerr));

            size_t x = increment > 0 ? i : _width - i - 1;
            _rawdest[y * _width + x] = remap[idx];

            int alpha = std::max(map[idx][3], _rawsrc[offset + 3]);
            int colorimp = 255 - ((255 - alpha) * (255 - alpha) / 255);

            int thisrerr = (map[idx][0] - _rawsrc[offset]) * colorimp / 255;
            int thisberr = (map[idx][1] - _rawsrc[offset + 1]) * colorimp / 255;
            int thisgerr = (map[idx][2] - _rawsrc[offset + 2]) * colorimp / 255;
            int thisaerr = map[idx][3] - _rawsrc[offset + 3];

            rerr  += thisrerr;
            gerr  += thisberr;
            berr  +=  thisgerr;
            aerr  += thisaerr;

            unsigned int thiserr = (thisrerr * thisrerr + thisberr * thisberr + thisgerr * thisgerr + thisaerr * thisaerr) * 2;
            floyderr = rerr * rerr + gerr * gerr + berr * berr + aerr * aerr;

            int L = 10;
            while (rerr * rerr > L * L || gerr * gerr > L * L || berr * berr > L * L || aerr * aerr > L * L ||
                   floyderr > thiserr || floyderr > L * L * 2)
            {
                rerr /=  2;
                gerr /=  2;
                berr /=  2;
                aerr /=  2;
                floyderr = rerr * rerr + gerr * gerr + berr * berr + aerr * aerr;
            }

            if (i > 0)
            {
                _rawsrc[nextoffset - increment + 3] = CLAMP(_rawsrc[nextoffset - increment + 3] - aerr * 3 / 16);
                _rawsrc[nextoffset - increment + 2] = CLAMP(_rawsrc[nextoffset - increment + 2] - berr * 3 / 16);
                _rawsrc[nextoffset - increment + 1] = CLAMP(_rawsrc[nextoffset - increment + 1] - gerr * 3 / 16);
                _rawsrc[nextoffset - increment]     = CLAMP(_rawsrc[nextoffset - increment]     - rerr * 3 / 16);
            }
            if (i + 1<_width)
            {
                _rawsrc[nextoffset + increment + 3] = CLAMP(_rawsrc[nextoffset + increment + 3] - aerr / 16);
                _rawsrc[nextoffset + increment + 2] = CLAMP(_rawsrc[nextoffset + increment + 2] - berr / 16);
                _rawsrc[nextoffset + increment + 1] = CLAMP(_rawsrc[nextoffset + increment + 1] - gerr / 16);
                _rawsrc[nextoffset + increment]     = CLAMP(_rawsrc[nextoffset + increment]     - rerr / 16);
            }
            _rawsrc[nextoffset + 3] = CLAMP(_rawsrc[nextoffset + 3] - aerr * 5 / 16);
            _rawsrc[nextoffset + 2] = CLAMP(_rawsrc[nextoffset + 2] - berr * 5 / 16 );
            _rawsrc[nextoffset + 1] = CLAMP(_rawsrc[nextoffset + 1] - gerr * 5 / 16);
            _rawsrc[nextoffset]     = CLAMP(_rawsrc[nextoffset]   - rerr * 5 / 16  );
        }

        rerr = rerr * 7 / 16;
        gerr = gerr * 7 / 16;
        berr = berr * 7 / 16;
        aerr = aerr * 7 / 16;
    }
}


ColorQuantizer::~ColorQuantizer()
{
    destroy();
}

void ColorQuantizer::destroy()
{
    if (_rawdest)
    {
        delete[] _rawdest;
    }
    if (_rawsrc)
    {
        delete[] _rawsrc;
    }
    if (_palette)
    {
        delete[] _palette;
    }
    if (_trans)
    {
        delete[] _trans;
    }
    delete[] _dest;
    delete[] _src;
};

void quantize_color(Image*& image, PNGWriter*& writer)
{
    ColorQuantizer quantizer(image->width(), image->height());
    quantizer.process(image->raw_image());
    writer->process(quantizer.delegate_rawimage(), quantizer.delegate_palette(), quantizer.delegate_trans());
}


#endif
