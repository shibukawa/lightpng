#include <iostream>
#include "MedianCut32bitQuantizer.h"
#include "libimagequant.h"

void MedianCut32bitQuantizer::_process()
{
    liq_attr *attr = liq_attr_create();
    liq_set_speed(attr, 1);
    liq_image *image = liq_image_create_rgba(attr, reinterpret_cast<void*>(_rawsrc.get()), _width, _height, 0);
    liq_result *res = liq_quantize_image(attr, image);

    liq_set_dithering_level(res, 1);
    liq_write_remapped_image(res, image, reinterpret_cast<void*>(_rawdest.get()), _width * _height);
    const liq_palette *pal = liq_get_palette(res);

    for (size_t i = 0; i < pal->count; ++i)
    {
        const liq_color& color  = pal->entries[i];
        _palette[i].red   = color.r;
        _palette[i].green = color.g;
        _palette[i].blue  = color.b;
        _trans[i]         = color.a;
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
