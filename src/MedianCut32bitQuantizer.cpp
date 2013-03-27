#include <iostream>
#include "MedianCut32bitQuantizer.h"
#include "libimagequant.h"

void MedianCut32bitQuantizer::process_()
{
    liq_attr *attr = liq_attr_create();
    liq_set_speed(attr, 1);
    liq_image *image = liq_image_create_rgba(attr, reinterpret_cast<void*>(rawsrc_.get()), width_, height_, 0);
    liq_result *res = liq_quantize_image(attr, image);

    liq_set_dithering_level(res, 1);
    liq_write_remapped_image(res, image, reinterpret_cast<void*>(rawdest_.get()), width_ * height_);
    const liq_palette *pal = liq_get_palette(res);

    for (size_t i = 0; i < pal->count; ++i)
    {
        const liq_color& color  = pal->entries[i];
        palette_[i].red   = color.r;
        palette_[i].green = color.g;
        palette_[i].blue  = color.b;
        trans_[i]         = color.a;
    }

    liq_attr_destroy(attr);
    liq_image_destroy(image);
    liq_result_destroy(res);
}

void median_cut_32bit_quantize(Image& image, PNGWriter& writer, bool hasAlphaChannel)
{
    MedianCut32bitQuantizer quantizer(image.width(), image.height());
    quantizer.process(image.image(), hasAlphaChannel);
    writer.process(quantizer.buffer(), quantizer.palette(), quantizer.trans(), true);
}
