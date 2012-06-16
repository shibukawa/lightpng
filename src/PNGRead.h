#ifndef PNGREAD_H
#define PNGREAD_H

#include <png.h>
#include "lightpng.h"
#include "Read.h"

class PNGRead : public Read
{
public:
    explicit PNGRead(const char* filepath);
    virtual ~PNGRead();

    size_t channels() const throw() { return _channels; }

private:
    png_structp _png;
    png_infop _info, _end;

    size_t _channels;

    void destroy();
};

#endif
