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

    bool hasAlpha() const { return hasAlpha_; };
    bool hasAlphaChannel() const { return channels_ == 4; }

    size_t channels() const throw() { return channels_; }

private:
    png_structp png_;
    png_infop info_, end_;

    bool hasAlpha_;
    size_t channels_;

    void checkHasAlpha();

    void destroy();
};

#endif
