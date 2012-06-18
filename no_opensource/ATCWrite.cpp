#include <iostream>
#include <cstdio>
#include <memory>
#include <zlib.h>
#include <png.h>
#include <TextureConverter.h>
#include "ATCWrite.h"
#include "ExpandRead.h"


TQonvertImage* CreateEmptyTexture()
{
    TQonvertImage* pTexture;

    pTexture = (TQonvertImage*)malloc(sizeof(TQonvertImage));
    memset(pTexture, 0, sizeof(TQonvertImage));
    pTexture->pFormatFlags = (TFormatFlags*)malloc(sizeof(TFormatFlags));
    memset(pTexture->pFormatFlags, 0, sizeof(TFormatFlags));

    return pTexture;
}

void FreeTexture(TQonvertImage* pTexture, bool removeData = false)
{
    free(pTexture->pFormatFlags);
    if (removeData)
    {
        delete[] pTexture->pData;
    }
    free(pTexture);
}

ATCWrite::~ATCWrite()
{
    destroy();
}

void ATCWrite::process(unsigned char* srcRawData, bool hasAlpha)
{
    _texture = CreateEmptyTexture();
    _texture->nWidth = _width;
    _texture->nHeight = _height;
    _texture->nDataSize = 0;
    _texture->pData = NULL; 

    TQonvertImage* srcImage = CreateEmptyTexture();
    srcImage->nFormat = Q_FORMAT_RGBA_8888;
    srcImage->nWidth = _width;
    srcImage->nHeight = _height;
    srcImage->nDataSize = _width * _height * 4;

    if (hasAlpha)
    {
        srcImage->pData = srcRawData;
        _texture->nFormat = Q_FORMAT_ATITC_RGBA;
        Qonvert(srcImage, _texture);
        _texture->pData = new unsigned char[_texture->nDataSize];
        Qonvert(srcImage, _texture);
    }
    else
    {
        _texture->nFormat = Q_FORMAT_ATITC_RGB;
        ExpandRead expander(_width, _height, srcRawData);
        srcImage->pData = expander.raw_buffer();
        Qonvert(srcImage, _texture);
        _texture->pData = new unsigned char[_texture->nDataSize];
        Qonvert(srcImage, _texture);
    }
    FreeTexture(srcImage);
};

void ATCWrite::write(const char* filepath)
{
    FILE* out = fopen(filepath, "w");
    if(out)
    {
        fwrite(_texture->pData, _texture->nDataSize, 1, out);
        fclose(out);
    }
};

void ATCWrite::writeWithHeader(const char* filepath)
{
    FILE* out = fopen(filepath, "w");
    if(out)
    {
        fwrite(_texture, sizeof(TQonvertImage), 1, out);
        fwrite(_texture->pData, _texture->nDataSize, 1, out);
        fclose(out);
    }
};

void ATCWrite::writeToPNG(const char* filepath)
{
    TQonvertImage* temp = CreateEmptyTexture();
    temp->nFormat = Q_FORMAT_RGBA_8888;
    temp->nWidth = _width;
    temp->nHeight = _height;
    temp->nDataSize = _width * _height * 4;
    Qonvert(_texture, temp);

    FreeTexture(temp, true);

    png_bytep raw_data = temp->pData;
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

void ATCWrite::destroy()
{
    if (_texture)
    {
        FreeTexture(_texture, true);
    }
};
