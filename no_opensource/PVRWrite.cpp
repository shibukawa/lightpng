#include <iostream>
#include <cstdio>
#include <zlib.h>
#include <png.h>
#include <PVRTextureUtilities.h>
#include "PVRWrite.h"
#include "ExpandRead.h"

using namespace pvrtexture;

PVRWrite::~PVRWrite()
{
    destroy();
}

void PVRWrite::process(unsigned char* src, bool hasAlpha)
{
    _header = new CPVRTextureHeader(PVRStandard8PixelType.PixelTypeID, _width, _height);
    if (hasAlpha)
    {
        PixelType PVRTC4BPP_RGBA(ePVRTPF_PVRTCI_4bpp_RGBA);
        _pvr = new CPVRTexture(*_header, src);
        Transcode(*_pvr, PVRTC4BPP_RGBA, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB, ePVRTCBest);
    }
    else
    {
        ExpandRead expander(_width, _height, src);
        PixelType PVRTC4BPP_RGB(ePVRTPF_PVRTCI_4bpp_RGB);
        _pvr = new CPVRTexture(*_header, expander.raw_buffer());
        Transcode(*_pvr, PVRTC4BPP_RGB, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB, ePVRTCBest);
    }
};

void PVRWrite::write(const char* filepath)
{
    _pvr->saveFile(filepath);
};

void PVRWrite::writeToLegacy(const char* filepath)
{
    _pvr->saveFileLegacyPVR(filepath, eOGLES2);
};

void PVRWrite::writeToPNG(const char* filepath)
{
    CPVRTexture rawImage(*_pvr);
    Transcode(rawImage, PVRStandard8PixelType, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB);
    png_bytep raw_data = reinterpret_cast<png_bytep>(rawImage.getDataPtr());
    png_bytepp raw_list = new png_bytep[_height];
    for (int i = 0; i < _height; ++i)
    {
        raw_list[i] = raw_data + (_width * i * 4);
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
    png_set_IHDR(png, info, _width, _height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png, info);
    png_write_image(png, raw_list);
    png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
    png_write_end(png, info);
    delete[] raw_list;
};

void PVRWrite::destroy()
{
    if (_pvr)
    {
        delete _pvr;
        _pvr = 0;
    }
    if (_header)
    {
        delete _header;
        _header = 0;
    }
};
