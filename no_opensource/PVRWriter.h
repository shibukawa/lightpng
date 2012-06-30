#ifndef PVRWRITER_H
#define PVRWRITER_H

namespace pvrtexture
{
    class CPVRTexture;
    class CPVRTextureHeader;
};

class PVRWriter
{
public:
    explicit PVRWriter(size_t width, size_t height)
        : _pvr(0), _header(0), _width(width), _height(height) {}
    virtual ~PVRWriter();

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
