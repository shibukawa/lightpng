#ifndef PNGREADER_H
#define PNGREADER_H

#include <png.h>
#include "LPType.h"
#include "lightpng.h"
#include "Image.h"

class PNGReader : public Image
{
public:
    explicit PNGReader(const char* filepath);
    virtual ~PNGReader();

    bool hasAlpha() const;
    bool hasAlphaChannel() const { return _channels == 4; }

    size_t channels() const throw() { return _channels; }

private:
    png_structp _png;
    png_infop _info, _end;

    size_t _channels;

    void destroy();
};

#endif
