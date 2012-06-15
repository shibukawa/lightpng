#ifndef PNGREAD_H
#define PNGREAD_H

#include <png.h>
#include "lightpng.h"

class PNGRead
{
public:
    explicit PNGRead(const char* filepath);
    virtual ~PNGRead();

    operator void*() const { return _valid ? const_cast<PNGRead*>(this) : 0; }
    bool operator!() const { return !_valid; }

    void write(const char* filepath);
    size_t width() const throw() { return _width; }
    size_t height() const throw() { return _height; }
    size_t channels() const throw() { return _channels; }
    png_bytepp raw_image() const { return _data; }

private:
    png_structp _png;
    png_infop _info, _end;

    png_bytepp _data;
    size_t _width, _height;
    size_t _channels;
    bool _valid;

    void destroy();
};

#endif
