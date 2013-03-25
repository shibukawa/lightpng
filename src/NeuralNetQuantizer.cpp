#include <iostream>
#include "NeuralNetQuantizer.h"

void NeuralNetQuantizer::_process()
{
    size_t bot_idx, top_idx;  // for remapping of indices
    size_t x;
    size_t remap[MAXNETSIZE];
    double r_sens = 1.0;
    double g_sens = 1.0;
    double b_sens = 1.0;
    double a_sens = 1.0;
    double remap_r_sens = 1.0;
    double remap_g_sens = 1.0;
    double remap_b_sens = 1.0;
    double remap_a_sens = 1.0;
    double force_gamma = 0;
    int force_alpha_class_correctness = 0;
    double alpha_class_correction = 0.0;
    int use_alpha_importance_heuristic = 1;
    double exclusion_threshold = 0.5;
    double quantization_gamma = 1.8;
    int colour_space = RGB;

    size_t y;
    unsigned char map[MAXNETSIZE][4];
    int newcolors = 256;

    // Start neuquant
    palinitnet(NULL, 0, 1.0, (unsigned char*)_rawsrc.get(), _height * _width * 4, newcolors,
        colour_space, quantization_gamma, alpha_class_correction,
        force_alpha_class_correctness, r_sens, g_sens, b_sens, a_sens,
        remap_r_sens, remap_g_sens, remap_b_sens, remap_a_sens,
        exclusion_threshold, use_alpha_importance_heuristic);

    int verbose = 0;
    int sample_factor = 1;
    double unisolate = 0.0;
    learn(sample_factor, unisolate, verbose);

    int strict_pal_rgba = 0;
    getcolormap((unsigned char*)map, strict_pal_rgba);
    size_t transnum = 0;

    // Remap indexes so all tRNS chunks are together
    //PNGNQ_MESSAGE("  Remapping colormap to eliminate opaque tRNS - chunk entries...\n");
    for (top_idx = newcolors - 1, x = 0;  x < newcolors;  ++x)
    {
        if (map[x][3] ==  255)  // maxval
        {
            remap[x] = top_idx--;
        }
        else
        {
            remap[x] = transnum++;
        }
    }

    // PNGNQ_MESSAGE( "%d entr%s left\n", bot_idx,(bot_idx ==  1)? "y" : "ies");

    // GRR TO DO:  if bot_idx ==  0, check whether all RGB samples are gray
    // and if so, whether grayscale sample_depth would be same
    // = > skip following palette section and go grayscale

    // Remap and make palette entries
    for (x = 0; x < newcolors; ++x)
    {
        _palette[remap[x]].red   = map[x][0];
        _palette[remap[x]].green = map[x][1];
        _palette[remap[x]].blue  = map[x][2];
        _trans[remap[x]]         = map[x][3];
    }
    remap_floyd(map, remap);
}

void NeuralNetQuantizer::remap_floyd(unsigned char map[MAXNETSIZE][4], size_t* remap)
{
    for (size_t y = 0; y < _height; ++y)
    {
        int offset, nextoffset;

        int rerr = 0;
        int berr = 0;
        int gerr = 0;
        int aerr = 0;

        offset = y * _width * 4;
        nextoffset = offset;
        if (y + 1 < _height)
        {
            nextoffset += _width * 4;
        }
        int increment = 4;

        for (size_t i = 0; i < _width; i++, offset += increment, nextoffset += increment)
        {
            int idx;
            unsigned int floyderr = rerr * rerr + gerr * gerr + berr * berr + aerr * aerr;

            size_t x = increment > 0 ? i : _width - i - 1;

            idx = inxsearch(CLAMP(_rawsrc[offset + 3] - aerr),
                            CLAMP(_rawsrc[offset + 2] - berr),
                            CLAMP(_rawsrc[offset + 1] - gerr),
                            CLAMP(_rawsrc[offset]     - rerr));

            _rawdest[y * _width + x] = remap[idx];

            int alpha = std::max(map[idx][3], _rawsrc[offset + 3]);
            int colorimp = 255 - ((255 - alpha) * (255 - alpha) / 255);

            int thisrerr = (map[idx][0] - _rawsrc[offset]) * colorimp / 255;
            int thisberr = (map[idx][1] - _rawsrc[offset + 1]) * colorimp / 255;
            int thisgerr = (map[idx][2] - _rawsrc[offset + 2]) * colorimp / 255;
            int thisaerr = map[idx][3] - _rawsrc[offset + 3];

            rerr += thisrerr;
            gerr += thisberr;
            berr += thisgerr;
            aerr += thisaerr;

            unsigned int thiserr = (thisrerr * thisrerr + thisberr * thisberr + thisgerr * thisgerr + thisaerr * thisaerr) * 2;
            floyderr = rerr * rerr + gerr * gerr + berr * berr + aerr * aerr;

            int L = 10;
            while (rerr * rerr > L * L || gerr * gerr > L * L || berr * berr > L * L || aerr * aerr > L * L ||
                   floyderr > thiserr || floyderr > L * L * 2)
            {
                rerr /= 2;
                gerr /= 2;
                berr /= 2;
                aerr /= 2;
                floyderr = rerr * rerr + gerr * gerr + berr * berr + aerr * aerr;
            }

            if (i > 0)
            {
                _rawsrc[nextoffset - increment + 3] = CLAMP(_rawsrc[nextoffset - increment + 3] - aerr * 3 / 16);
                _rawsrc[nextoffset - increment + 2] = CLAMP(_rawsrc[nextoffset - increment + 2] - berr * 3 / 16);
                _rawsrc[nextoffset - increment + 1] = CLAMP(_rawsrc[nextoffset - increment + 1] - gerr * 3 / 16);
                _rawsrc[nextoffset - increment]     = CLAMP(_rawsrc[nextoffset - increment]     - rerr * 3 / 16);
            }
            if (i + 1 < _width)
            {
                _rawsrc[nextoffset + increment + 3] = CLAMP(_rawsrc[nextoffset + increment + 3] - aerr / 16);
                _rawsrc[nextoffset + increment + 2] = CLAMP(_rawsrc[nextoffset + increment + 2] - berr / 16);
                _rawsrc[nextoffset + increment + 1] = CLAMP(_rawsrc[nextoffset + increment + 1] - gerr / 16);
                _rawsrc[nextoffset + increment]     = CLAMP(_rawsrc[nextoffset + increment]     - rerr / 16);
            }
            _rawsrc[nextoffset + 3] = CLAMP(_rawsrc[nextoffset + 3] - aerr * 5 / 16);
            _rawsrc[nextoffset + 2] = CLAMP(_rawsrc[nextoffset + 2] - berr * 5 / 16);
            _rawsrc[nextoffset + 1] = CLAMP(_rawsrc[nextoffset + 1] - gerr * 5 / 16);
            _rawsrc[nextoffset]     = CLAMP(_rawsrc[nextoffset]     - rerr * 5 / 16);
        }

        rerr = rerr * 7 / 16;
        gerr = gerr * 7 / 16;
        berr = berr * 7 / 16;
        aerr = aerr * 7 / 16;
    }
}

void NeuralNetQuantizer::remap_simple(unsigned char map[MAXNETSIZE][4], size_t* remap)
{
    for (size_t y = 0; y < _height; ++y)
    {
        size_t offset = y * _width * 4;
        for(size_t x = 0; x < _width; x++)
        {
            _rawdest[y * _width + x] = remap[inxsearch(_rawsrc[x * 4 + offset + 3],
                                                       _rawsrc[x * 4 + offset + 2],
                                                       _rawsrc[x * 4 + offset + 1],
                                                       _rawsrc[x * 4 + offset])];
        }
    }
}


void neural_net_quantize(Image& image, PNGWriter& writer, bool hasAlphaChannel)
{
    NeuralNetQuantizer quantizer(image.width(), image.height());
    quantizer.process(image.image(), hasAlphaChannel);
    writer.process(quantizer.buffer(), quantizer.palette(), quantizer.trans(), true);
}
