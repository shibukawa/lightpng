#include <iostream>
#include <cstdio>
#include <memory>
#include <zlib.h>
#include <png.h>
#include <TextureConverter.h>
#include "ATCWriter.h"
#include "BitChanger.h"


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

ATCWriter::~ATCWriter()
{
    destroy();
}

void ATCWriter::process(buffer_t srcRawData, bool hasAlpha)
{
    BitChanger expander(width_, height_, hasAlpha, srcRawData);
    size_ = expander.height();

    texture_ = CreateEmptyTexture();
    texture_->nWidth = size_;
    texture_->nHeight = size_;
    texture_->nDataSize = 0;
    texture_->pData = NULL; 

    TQonvertImage* srcImage = CreateEmptyTexture();
    srcImage->nFormat = Q_FORMAT_RGBA_8888;
    srcImage->nWidth = size_;
    srcImage->nHeight = size_;
    srcImage->nDataSize = size_ * size_ * 4;
    srcImage->pData = expander.buffer().get();

    if (hasAlpha)
    {
        texture_->nFormat = Q_FORMAT_ATITC_RGBA;
        Qonvert(srcImage, texture_);
        texture_->pData = new unsigned char[texture_->nDataSize];
        Qonvert(srcImage, texture_);
    }
    else
    {
        texture_->nFormat = Q_FORMAT_ATITC_RGB;
        Qonvert(srcImage, texture_);
        texture_->pData = new unsigned char[texture_->nDataSize];
        Qonvert(srcImage, texture_);
    }
    FreeTexture(srcImage);
};

void ATCWriter::write(const char* filepath)
{
    FILE* out = fopen(filepath, "w");
    if(out)
    {
        fwrite(texture_->pData, texture_->nDataSize, 1, out);
        fclose(out);
    }
};

void ATCWriter::writeWithHeader(const char* filepath)
{
    FILE* out = fopen(filepath, "w");
    if(out)
    {
        int format = texture_->nFormat;
        if (texture_->nFormat == Q_FORMAT_ATITC_RGBA)
        {
            texture_->nFormat = 20;
        }
        else
        {
            texture_->nFormat = 21;
        }
        fwrite(texture_, sizeof(TQonvertImage), 1, out);
        fwrite(texture_->pData, texture_->nDataSize, 1, out);
        fclose(out);
        texture_->nFormat = format;
    }
};

void ATCWriter::writeToPNG(const char* filepath)
{
    TQonvertImage* temp = CreateEmptyTexture();
    temp->nFormat = Q_FORMAT_RGBA_8888;
    temp->nWidth = size_;
    temp->nHeight = size_;
    temp->nDataSize = size_ * size_ * 4;
    temp->pData = new unsigned char[temp->nDataSize];
    Qonvert(texture_, temp);

    png_bytep raw_data = temp->pData;
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
    png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
    png_write_end(png, info);
    FreeTexture(temp, true);
    delete[] raw_list;
};

void ATCWriter::destroy()
{
    if (texture_)
    {
        FreeTexture(texture_, true);
    }
};
