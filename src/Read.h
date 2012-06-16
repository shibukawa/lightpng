#ifndef READ_H
#define READ_H

class Read
{
public:
    size_t width() const throw() { return _width; }
    size_t height() const throw() { return _height; }
    unsigned char** raw_image() const { return _data; }

    operator void*() const { return _valid ? const_cast<Read*>(this) : 0; }
    bool operator!() const { return !_valid; }
    bool valid() const { return _valid; }

    virtual ~Read() {}

protected:
    explicit Read() : _data(0), _width(0), _height(0), _valid(false) {}

    unsigned char** _data;
    size_t _width, _height;
    bool _valid;
};

#endif
