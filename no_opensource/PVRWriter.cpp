#include <iostream>
#include <cstdio>
#include <zlib.h>
#include <png.h>
#include <PVRTextureUtilities.h>
#include "PVRWriter.h"
#include "BitChanger.h"

using namespace pvrtexture;

PVRWriter::~PVRWriter()
{
    destroy();
}

void PVRWriter::process(buffer_t src, bool hasAlpha)
{
    bool result;
    BitChanger expander(width_, height_, hasAlpha, src);
    size_ = expander.height();
    CPVRTextureHeader header(PVRStandard8PixelType.PixelTypeID, size_, size_);
    if (hasAlpha)
    {
        PixelType PVRTC4BPP_RGBA(ePVRTPF_PVRTCI_4bpp_RGBA);
        pvr_ = new CPVRTexture(header, expander.buffer().get());
        result = Transcode(*pvr_, PVRTC4BPP_RGBA, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB);
    }
    else
    {
        PixelType PVRTC4BPP_RGB(ePVRTPF_PVRTCI_4bpp_RGB);
        pvr_ = new CPVRTexture(header, expander.buffer().get());
        result = Transcode(*pvr_, PVRTC4BPP_RGB, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB);
    }
};

void PVRWriter::write(const char* filepath)
{
    pvr_->saveFile(filepath);
};

void PVRWriter::writeToLegacy(const char* filepath)
{
    pvr_->saveFileLegacyPVR(filepath, eOGLES2);
};

void PVRWriter::writeToPNG(const char* filepath)
{
    CPVRTexture rawImage(*pvr_);
    Transcode(rawImage, PVRStandard8PixelType, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB);
    png_bytep raw_data = reinterpret_cast<png_bytep>(rawImage.getDataPtr());
    png_bytepp raw_list = new png_bytep[height_];
    for (size_t i = 0; i < height_; ++i)
    {
        raw_list[i] = raw_data + (size_ * i * 4);
    }
    FILE *fp = std::fopen(filepath, "wb");
    if (!fp)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return;
    }
    png_structp png = 0;
    png_infop info = 0;
    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return;
    }
    info = png_create_info_struct(png);
    if (!info)
    {
        std::cout << "Internal Error: " << filepath << std::endl;
        return;
    }
    png_init_io(png, fp);
    png_set_compression_level(png, Z_BEST_COMPRESSION);
    png_set_IHDR(png, info, width_, height_, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png, info);
    png_write_image(png, raw_list);
    png_write_end(png, info);
    delete[] raw_list;
};

void PVRWriter::destroy()
{
    if (pvr_)
    {
        delete pvr_;
        pvr_ = 0;
    }
};
