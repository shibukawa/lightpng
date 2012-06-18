#ifndef PVRWRITE_H
#define PVRWRITE_H

namespace pvrtexture
{
    class CPVRTexture;
    class CPVRTextureHeader;
};

class PVRWrite
{
public:
    explicit PVRWrite(size_t width, size_t height)
        : _pvr(0), _header(0), _width(width), _height(height) {}
    virtual ~PVRWrite();

    void process(unsigned char* src, bool hasAlpha);
    void write(const char* filepath);
    void writeToLegacy(const char* filepath);
    void writeToPNG(const char* filepath);

private:
    pvrtexture::CPVRTexture* _pvr;
    pvrtexture::CPVRTextureHeader* _header;
    size_t _width, _height;

    void destroy();
};

#endif
