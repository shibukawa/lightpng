#include <iostream>
#include <sys/time.h>
#include <zlib.h>
#include <png.h>
#include <boost/scoped_ptr.hpp>
#include "lightpng.h"
#include "Image.h"
#include "PNGReader.h"
#include "JPEGReader.h"
#include "ReduceColor.h"
#include "MedianCut16bitQuantizer.h"
#include "MedianCut32bitQuantizer.h"
#include "PNGWriter.h"

void help()
{
    std::cout << "lightpng: PNG optimization tool for game graphics" << std::endl
              << "   Copyright (c) 2012-2013 Yoshiki Shibukawa (DeNA Co.,Ltd, and ngmoco LLC)" << std::endl
              << std::endl
              << "   Open source version" << std::endl
              << std::endl
              << "Texture file processing tool to use for texture." << std::endl
              << "  * Generetes small file size 16 bit PNG file." << std::endl
              << "  * It can generate emulated preview image in PNG format." << std::endl
              << std::endl
              << "usage> lightpng [options] input_image.png/jpg [output options]" << std::endl
              << std::endl
              << "  [options]" << std::endl
              << "   -o, --optimize [level] : Select optimization level" << std::endl
              << "                          : 0 - No optimize (fastest)" << std::endl
              << "                          : 1 - PNG zlib option optimize + index color optimize(default)" << std::endl
              << "                          : 2 - Use zopfli with one fiilter + 1" << std::endl
              << "                          : 3 - Use zopfli with all filters + 1" << std::endl
              << "   -b, --benchmark        : Display compression time" << std::endl
              << "   -v, --verbose          : Display compression result" << std::endl
              << "   -h, --help             : Show this message" << std::endl
              << std::endl
              << "  [output options]" << std::endl
              << "   -16a PATH     : 16 bit PNG with 4 bit alpha (RGBA 4444)" << std::endl
              << "    or -16 PATH  : If source image doesn't have alpha, it generates RGB 565 PNG." << std::endl
              << "   -16m PATH     : 16 bit PNG with 1 bit alpha (RGBA 5551)" << std::endl
              << "                 : If source image doesn't have alpha, it generates RGB 565 PNG." << std::endl
              << "   -16i PATH     : 16 bit indexed PNG with 4 bit alpha (RGBA 4444)" << std::endl
              << "                 : If source image doesn't have alpha, it generates RGB 565 PNG." << std::endl
              << "   -32 PATH      : 24/32 bit PNG" << std::endl
              << "   -32i PATH     : Indexed 24/32 bit PNG" << std::endl
              << "   -p16a PATH    : 16 bit PNG with 4 bit alpha (RGBA 4444) preview file" << std::endl
              << "    or -p16 PATH" << std::endl
              << "   -p16i PATH    : 16 bit Indexed PNG with 4 bit alpha (RGBA 4444) preview file" << std::endl
              << "   -p16m PATH    : 16 bit PNG with 1 bit alpha (RGBA 5551) preview file" << std::endl
              << std::endl;
}

bool check_ext(std::string filename, const char* ext)
{
    std::string::size_type pos(filename.rfind('.'));
    if (pos != std::string::npos)
    {
        std::string fileext = filename.substr(pos, filename.length());
        return (fileext == ext);
    }
    return false;
}


double get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}


void parse_arg(int argc, const char** argv, const char*& input, output_list& outputs, size_t& optimize, bool& bench, bool& verbose, Mode& mode, InputFileType& inputType)
{
    int state = 0;
    for (int i = 1; i < argc; ++i)
    {
        std::string opt(argv[i]);
        if (opt == "-h" or opt == "--help")
        {
            mode = helpMode;
            break;
        }
        if (state == 0)
        {
            if (opt == "-o" or opt == "--optimize")
            {
                ++i;
                if (i == argc)
                {
                    std::cout << opt << " needs optimize level (0-3)." << std::endl;
                    mode = helpMode;
                    break;
                }
                std::string level(argv[i]);
                if (level == "0")
                {
                    optimize = 0;
                }
                else if (level == "1")
                {
                    optimize = 1;
                }
                else if (level == "2")
                {
                    optimize = 2;
                }
                else if (level == "3")
                {
                    optimize = 3;
                }
                else
                {
                    std::cout << " Optimization level should be 0-3, but was " << level << "." << std::endl;
                    mode = helpMode;
                    break;
                }
            }
            else if (opt == "-b" or opt == "--benchmark")
            {
                bench = true;
            }
            else if (opt == "-v" or opt == "--verbose")
            {
                verbose = true;
            }
            else if (check_ext(opt, ".png"))
            {
                input = argv[i];
                inputType = PNGFile;
                state = 1;
            }
            else if (check_ext(opt, ".jpg") or check_ext(opt, ".jpeg"))
            {
                input = argv[i];
                inputType = JPEGFile;
                state = 1;
            }
            else
            {
                std::cout << "Unknown Parameter: " << opt << std::endl;
                mode = helpMode;
                break;
            }
        }
        else if (state == 1)
        {
            if ((opt == "-16a") or (opt == "-p16a") or (opt == "-16") or (opt == "-p16"))
            {
                i++;
                if (i == argc)
                {
                    std::cout << opt << " needs file path" << std::endl;
                    mode = helpMode;
                    break;
                }
                std::string path(argv[i]);
                if (check_ext(path, ".png"))
                {
                    if (opt == "-16a" or opt == "-16")
                    {
                        outputs.push_back(output_type(AlphaPNGFile, path));
                    }
                    else
                    {
                        outputs.push_back(output_type(PreviewAlphaPNGFile, path));
                    }
                }
                else
                {
                    std::cout << opt << " file should be .png " << path << std::endl;
                    mode = helpMode;
                    break;
                }
            }
            else if ((opt == "-16m") or (opt == "-p16m"))
            {
                i++;
                if (i == argc)
                {
                    std::cout << opt << " needs file path" << std::endl;
                    mode = helpMode;
                    break;
                }
                std::string path(argv[i]);
                if (check_ext(path, ".png"))
                {
                    if (opt == "-16m")
                    {
                        outputs.push_back(output_type(MaskPNGFile, path));
                    }
                    else
                    {
                        outputs.push_back(output_type(PreviewMaskPNGFile, path));
                    }
                }
                else
                {
                    std::cout << opt << " file should be .png " << path << std::endl;
                    mode = helpMode;
                    break;
                }
            }
            else if ((opt == "-16i") or (opt == "-p16i"))
            {
                i++;
                if (i == argc)
                {
                    std::cout << opt << " needs file path" << std::endl;
                    mode = helpMode;
                    break;
                }
                std::string path(argv[i]);
                if (check_ext(path, ".png"))
                {
                    if (opt == "-16i")
                    {
                        outputs.push_back(output_type(IndexedReducedColorPNGFile, path));
                    }
                    else
                    {
                        outputs.push_back(output_type(PreviewIndexedReducedColorPNGFile, path));
                    }
                }
                else
                {
                    std::cout << opt << " file should be .png " << path << std::endl;
                    mode = helpMode;
                    break;
                }
            }
            else if (opt == "-32")
            {
                i++;
                if (i == argc)
                {
                    std::cout << "-32 needs file path" << std::endl;
                    mode = helpMode;
                    break;
                }
                std::string path(argv[i]);
                if (check_ext(path, ".png"))
                {
                    outputs.push_back(output_type(FullColorPNGFile, path));
                }
                else
                {
                    std::cout << "-32 file should be .png " << path << std::endl;
                    mode = helpMode;
                    break;
                }
            }
            else if (opt == "-32i")
            {
                i++;
                if (i == argc)
                {
                    std::cout << "-32i needs file path" << std::endl;
                    mode = helpMode;
                    break;
                }
                std::string path(argv[i]);
                if (check_ext(path, ".png"))
                {
                    outputs.push_back(output_type(IndexedColorPNGFile, path));
                }
                else
                {
                    std::cout << "-32i file should be .png " << path << std::endl;
                    mode = helpMode;
                    break;
                }
            }
            else
            {
                std::cout << "Unknown Parameter: " << opt << std::endl;
                mode = helpMode;
                break;
            }
        }
    }
}


void process_image(const char*& input_path, output_list& outputs, size_t optimize, bool bench, bool verbose, Mode& mode, InputFileType& inputType)
{
    Image* reader;
    bool hasAlphaChannel = false;
    bool hasAlpha = false;
    if (inputType == PNGFile)
    {
        reader = new PNGReader(input_path);
        hasAlphaChannel = reader->hasAlphaChannel();
        hasAlpha = reader->hasAlpha();
    }
    else
    {
        reader = new JPEGReader(input_path);
    }

    if (reader->valid())
    {
        boost::scoped_ptr<PNGWriter> mask_png_writer;
        boost::scoped_ptr<PNGWriter> alpha_png_writer;
        boost::scoped_ptr<PNGWriter> noalpha_png_writer;
        boost::scoped_ptr<PNGWriter> preview_mask_png_writer;
        boost::scoped_ptr<PNGWriter> preview_alpha_png_writer;
        boost::scoped_ptr<PNGWriter> preview_noalpha_png_writer;
        boost::scoped_ptr<PNGWriter> fullcolor_png_writer;
        boost::scoped_ptr<PNGWriter> indexed_color_png_writer;
        boost::scoped_ptr<PNGWriter> indexed_reduced_color_png_writer;
        boost::scoped_ptr<PNGWriter> preview_indexed_reduced_color_png_writer;
        buffer_t image565;
        buffer_t image565p;
        buffer_t image5551;
        buffer_t image5551p;
        buffer_t image4444;
        buffer_t image4444p;

        for (output_list::iterator i = outputs.begin(); i != outputs.end(); ++i)
        {
            OutputFileType outputType = (*i).first;
            switch (outputType)
            {
            case MaskPNGFile:
                if (hasAlpha)
                {
                    if (!mask_png_writer)
                    {
                        mask_png_writer.reset(new PNGWriter(*reader, hasAlpha, optimize, verbose));
                        double t1 = get_time();
                        if (!image5551)
                        {
                            image5551 = reduce_color<1>(*reader, 5, 5, 5, true, false);
                        }
                        mask_png_writer->process(image5551);
                        if (bench)
                        {
                            double t2 = get_time();
                            std::cout << "16bit PNG compression: " << t2 - t1 << std::endl;
                        }
                    }
                    mask_png_writer->write((*i).second.c_str());
                }
                else
                {
                    if (!noalpha_png_writer)
                    {
                        noalpha_png_writer.reset(new PNGWriter(*reader, hasAlpha, optimize, verbose));
                        double t1 = get_time();
                        if (!image565)
                        {
                            image565 = reduce_color<0>(*reader, 5, 6, 5, hasAlphaChannel, false);
                        }
                        noalpha_png_writer->process(image565);
                        if (bench)
                        {
                            double t2 = get_time();
                            std::cout << "16bit PNG compression: " << t2 - t1 << std::endl;
                        }
                    }
                    noalpha_png_writer->write((*i).second.c_str());
                }
                break;
            case PreviewMaskPNGFile:
                if (hasAlpha)
                {
                    if (!preview_mask_png_writer)
                    {
                        preview_mask_png_writer.reset(new PNGWriter(*reader, hasAlpha, 0, verbose));
                        double t1 = get_time();
                        if (!image5551p)
                        {
                            image5551p = reduce_color<1>(*reader, 5, 5, 5, true, true);
                        }

                        preview_mask_png_writer->process(image5551p);
                        if (bench)
                        {
                            double t2 = get_time();
                            std::cout << "16bit PNG compression(Preview): " << t2 - t1 << std::endl;
                        }
                    }
                    preview_mask_png_writer->write((*i).second.c_str());
                }
                else
                {
                    if (!preview_noalpha_png_writer)
                    {
                        preview_noalpha_png_writer.reset(new PNGWriter(*reader, hasAlpha, 0, verbose));
                        double t1 = get_time();
                        if (!image565p)
                        {
                            image565p = reduce_color<0>(*reader, 5, 6, 5, hasAlphaChannel, true);
                        }
                        preview_noalpha_png_writer->process(image565p);
                        if (bench)
                        {
                            double t2 = get_time();
                            std::cout << "16bit PNG compression(Preview): " << t2 - t1 << std::endl;
                        }
                    }
                    preview_noalpha_png_writer->write((*i).second.c_str());
                }
                break;
            case AlphaPNGFile:
                if (hasAlpha)
                {
                    if (!alpha_png_writer)
                    {
                        alpha_png_writer.reset(new PNGWriter(*reader, hasAlpha, optimize, verbose));
                        double t1 = get_time();
                        if (!image4444)
                        {
                            image4444 = reduce_color<4>(*reader, 4, 4, 4, true, false);
                        }
                        alpha_png_writer->process(image4444);
                        if (bench)
                        {
                            double t2 = get_time();
                            std::cout << "16bit PNG compression: " << t2 - t1 << std::endl;
                        }
                    }
                    alpha_png_writer->write((*i).second.c_str());
                }
                else
                {
                    if (!noalpha_png_writer)
                    {
                        noalpha_png_writer.reset(new PNGWriter(*reader, hasAlpha, optimize, verbose));
                        double t1 = get_time();
                        if (!image565)
                        {
                            image565 = reduce_color<0>(*reader, 5, 6, 5, hasAlphaChannel, false);
                        }
                        noalpha_png_writer->process(image565);
                        if (bench)
                        {
                            double t2 = get_time();
                            std::cout << "16bit PNG compression: " << t2 - t1 << std::endl;
                        }
                    }
                    noalpha_png_writer->write((*i).second.c_str());
                }
                break;
            case PreviewAlphaPNGFile:
                if (hasAlpha)
                {
                    if (!preview_alpha_png_writer)
                    {
                        preview_alpha_png_writer.reset(new PNGWriter(*reader, hasAlpha, 0, verbose));
                        double t1 = get_time();
                        if (!image4444p)
                        {
                            image4444p = reduce_color<4>(*reader, 4, 4, 4, true, true);
                        }
                        preview_alpha_png_writer->process(image4444p);
                        if (bench)
                        {
                            double t2 = get_time();
                            std::cout << "16bit PNG compression(Preview): " << t2 - t1 << std::endl;
                        }
                    }
                    preview_alpha_png_writer->write((*i).second.c_str());
                }
                else
                {
                    if (!preview_noalpha_png_writer)
                    {
                        preview_noalpha_png_writer.reset(new PNGWriter(*reader, hasAlpha, 0, verbose));
                        double t1 = get_time();
                        if (!image565p)
                        {
                            image565p = reduce_color<0>(*reader, 5, 6, 5, hasAlphaChannel, true);
                        }
                        preview_noalpha_png_writer->process(image565p);
                        if (bench)
                        {
                            double t2 = get_time();
                            std::cout << "16bit PNG compression(Preview): " << t2 - t1 << std::endl;
                        }
                    }
                    preview_noalpha_png_writer->write((*i).second.c_str());
                }
                break;
            case IndexedReducedColorPNGFile:
                if (!indexed_reduced_color_png_writer)
                {
                    indexed_reduced_color_png_writer.reset(new PNGWriter(*reader, hasAlpha, optimize, verbose));
                    double t1 = get_time();
                    median_cut_16bit_quantize(*reader, *indexed_reduced_color_png_writer, hasAlphaChannel, hasAlpha, false);
                    if (bench)
                    {
                        double t2 = get_time();
                        std::cout << "16bit Indexed PNG compression: " << t2 - t1 << std::endl;
                    }
                }
                indexed_reduced_color_png_writer->write((*i).second.c_str());
                break;
            case PreviewIndexedReducedColorPNGFile:
                if (!preview_indexed_reduced_color_png_writer)
                {
                    preview_indexed_reduced_color_png_writer.reset(new PNGWriter(*reader, hasAlpha, 0, verbose));
                    double t1 = get_time();
                    median_cut_16bit_quantize(*reader, *preview_indexed_reduced_color_png_writer, hasAlphaChannel, hasAlpha, true);
                    if (bench)
                    {
                        double t2 = get_time();
                        std::cout << "16bit Indexed PNG compression(Preview): " << t2 - t1 << std::endl;
                    }
                }
                preview_indexed_reduced_color_png_writer->write((*i).second.c_str());
                break;
            case FullColorPNGFile:
                if (!fullcolor_png_writer)
                {
                    fullcolor_png_writer.reset(new PNGWriter(*reader, hasAlpha, optimize, verbose));
                    double t1 = get_time();
                    fullcolor_png_writer->process(reader->buffer(), !hasAlpha && hasAlphaChannel);
                    if (bench)
                    {
                        double t2 = get_time();
                        std::cout << "32bit PNG compression: " << t2 - t1 << std::endl;
                    }
                }
                fullcolor_png_writer->write((*i).second.c_str());
                break;
            case IndexedColorPNGFile:
                if (!indexed_color_png_writer)
                {
                    indexed_color_png_writer.reset(new PNGWriter(*reader, hasAlpha, optimize, verbose));
                    double t1 = get_time();
                    median_cut_32bit_quantize(*reader, *indexed_color_png_writer, hasAlphaChannel);
                    if (bench)
                    {
                        double t2 = get_time();
                        std::cout << "32bit Indexed PNG compression: " << t2 - t1 << std::endl;
                    }
                }
                indexed_color_png_writer->write((*i).second.c_str());
                break;
            }
        }
    }
    else
    {
        std::cout << "Read error" << std::endl;
    }
}

int main(int argc, const char** argv)
{
    const char* input_path = 0;
    output_list outputs;
    Mode mode = textureMode;
    InputFileType inputType;
    size_t optimize = 1;
    bool bench = false;
    bool verbose = false;
    int result;

    parse_arg(argc, argv, input_path, outputs, optimize, bench, verbose, mode, inputType);

    if (input_path == 0 || outputs.size() == 0)
    {
        mode = helpMode;
    }

    if (mode == helpMode)
    {
        help();
        result = 1;
    }
    else
    {
        process_image(input_path, outputs, optimize, bench, verbose, mode, inputType);
        result = 0;
    }
    return result;
}
