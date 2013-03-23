#ifndef IMAGE_H
#define IMAGE_H

#include "LPType.h"

class Image
{
public:
    size_t width() const throw() { return _width; }
    size_t height() const throw() { return _height; }
    rows_t image() const { return _rows; }
    buffer_t buffer() const { return _data; }

    virtual bool hasAlpha() const { return false; }
    virtual bool hasAlphaChannel() const { return false; }

    operator void*() const { return _valid ? const_cast<Image*>(this) : 0; }
    bool operator!() const { return !_valid; }
    bool valid() const { return _valid; }

    virtual ~Image() {}

protected:
    explicit Image() : _width(0), _height(0), _valid(false) {}

    void alloc(size_t pixelBytes)
    {
        _data.reset(new unsigned char[pixelBytes * _width * _height]);
        _rows.reset(new unsigned char*[_height]);
        for (size_t i = 0; i < _height; ++i)
        {
            _rows[i] = _data.get() + (i * _width * pixelBytes);
        }
    }
    buffer_t _data;
    rows_t _rows;
    size_t _width, _height;
    bool _valid;
};

#endif
