#ifndef QUANTIZER_H
#define QUANTIZER_H

#include <iostream>
#include "LPType.h"
#include "Image.h"


class Quantizer
{
public:
    explicit Quantizer(size_t width, size_t height)
    	: width_(width), height_(height)
	{
	    rawsrc_.reset(new unsigned char[4 * width * height]);
	    rawdest_.reset(new unsigned char[width * height]);
	    src_.reset(new unsigned char*[height]);
	    for (size_t i = 0; i < height_; ++i)
	    {
	        src_[i] = rawsrc_.get() + (i * width * 4);
	    }
	    palette_.reset(new png_color[256]);
	    trans_.reset(new unsigned char[256]);
	};
    virtual ~Quantizer() {}

    void process(rows_t src, bool hasAlphaChannel)
    {
        if (hasAlphaChannel)
	    {
	    	Image::copy_4_to_4(width_, height_, src, src_);
	    }
	    else
	    {
	    	Image::copy_3_to_4(width_, height_, src, src_);
	    }
	    process_();
	}
    void process(buffer_t src, bool hasAlphaChannel)
    {
        if (hasAlphaChannel)
	    {
	    	Image::copy_4_to_4(width_, height_, src, rawsrc_);
	    }
	    else
	    {
	    	Image::copy_3_to_4(width_, height_, src, rawsrc_);
	    }
	    process_();
	}
    buffer_t buffer()
    {
        return rawdest_;
    };
    palette_t palette()
    {
        return palette_;
    };
    trans_t trans()
    {
        return trans_;
    }

protected:
    rows_t src_;
    buffer_t rawsrc_;
    buffer_t rawdest_;
    size_t width_, height_;
    palette_t palette_;
    trans_t trans_;
    virtual void process_() {}
};

#endif

