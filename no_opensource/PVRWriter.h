#ifndef PVRWRITER_H
#define PVRWRITER_H

#include "LPType.h"

namespace pvrtexture
{
    class CPVRTexture;
    class CPVRTextureHeader;
};

class PVRWriter
{
public:
    explicit PVRWriter(size_t width, size_t height)
        : pvr_(0), header_(0), width_(width), height_(height), size_(0) {}
    virtual ~PVRWriter();

    void process(buffer_t src, bool hasAlpha);
    void write(const char* filepath);
    void writeToLegacy(const char* filepath);
    void writeToPNG(const char* filepath);

private:
    pvrtexture::CPVRTexture* pvr_;
    pvrtexture::CPVRTextureHeader* header_;
    size_t width_, height_, size_;

    void destroy();
};

#endif
