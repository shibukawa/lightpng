#ifndef READ_H
#define READ_H

class Read
{
public:
    size_t width() const throw() { return _width; }
    size_t height() const throw() { return _height; }
    unsigned char** raw_image() const { return _rows; }
    unsigned char* raw_buffer() const { return _data; }

    operator void*() const { return _valid ? const_cast<Read*>(this) : 0; }
    bool operator!() const { return !_valid; }
    bool valid() const { return _valid; }

    virtual ~Read() {
        if (_data)
        {
            delete[] _data;
        }
        if (_rows)
        {
            delete[] _rows;
        }
    }

protected:
    explicit Read() : _data(0), _rows(0), _width(0), _height(0), _valid(false) {}

    void alloc(size_t pixelBytes)
    {
        _data = new unsigned char[pixelBytes * _width * _height];
        _rows = new unsigned char*[_height];
        for (int i = 0; i < _height; ++i)
        {
            _rows[i] = _data + (i * _width * pixelBytes);
        }
    }
    unsigned char* _data;
    unsigned char** _rows;
    size_t _width, _height;
    bool _valid;
};

#endif
