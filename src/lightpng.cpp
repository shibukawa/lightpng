#include <iostream>
#include <zlib.h>
#include <png.h>
#include "lightpng.h"
#include "Image.h"
#include "PNGReader.h"
#include "JPEGReader.h"
#include "ReduceColor.h"
#include "PNGWriter.h"

void help()
{
    std::cout << "lightpng: PNG optimization tool for game graphics" << std::endl
              << "   Copyright (c) 2012 Yoshiki Shibukawa (DeNA Co.,Ltd, and ngmoco LLC)" << std::endl
              << "   Open source version" << std::endl
              << std::endl
              << "Texture file processing tool to use for texture." << std::endl
              << "  * Generetes small file size 16 bit PNG file." << std::endl
              << "  * It can generate emulated preview image in PNG format." << std::endl
              << std::endl
              << "usage> lightpng [options] input_image.png/jpg [output options]" << std::endl
              << std::endl
              << "  [options]" << std::endl
              << "   -t, --texture : Texture Mode (default)" << std::endl
              << "   -p, --preview : Preview Mode. All images are generated in PNG format." << std::endl
              << "   -h, --help    : Show this message" << std::endl
              << std::endl
              << "  [output options]" << std::endl
              << "   -16m PATH    : 16 bit PNG with 1 bit alpha (RGBA 5551)" << std::endl
              << "                : If source image doesn't have alpha, it generates RGB 565 PNG." << std::endl
              << "   -16a PATH    : 16 bit PNG with 4 bit alpha (RGBA 4444)" << std::endl
              << "                : If source image doesn't have alpha, it generates RGB 565 PNG." << std::endl
              << "   -32 PATH     : 24/32 bit PNG" << std::endl
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


void parse_arg(int argc, const char** argv, const char*& input, output_list& outputs, Mode& mode, FileType& inputType)
{
    int state = 0;
    for (int i = 1; i < argc; ++i)
    {
        std::string opt(argv[i]);
        if (opt == "-h" || opt == "--help")
        {
            mode = helpMode;
            break;
        }
        if (state == 0)
        {
            if (opt == "-p" || opt == "--preview")
            {
                mode = previewMode;
            }
            else if (opt == "-t" || opt == "--texture")
            {
                mode = textureMode;
            }
            else if (check_ext(opt, ".png"))
            {
                input = argv[i];
                inputType = PNGFile;
                state = 1;
            }
            else if (check_ext(opt, ".jpg") || check_ext(opt, ".jpeg"))
            {
                input = argv[i];
                inputType = JPEGFile;
                state = 1;
            }
            else
            {
                std::cout << "Unknow Parameter: " << opt << std::endl;
                mode = helpMode;
                break;
            }
        }
        else if (state == 1)
        {
            if (opt == "-16a")
            {
                i++;
                if (i == argc)
                {
                    std::cout << "-16a needs file path" << std::endl;
                    mode = helpMode;
                    break;
                }
                std::string path(argv[i]);
                if (check_ext(path, ".png"))
                {
                    outputs.push_back(output_type(AlphaPNGFile, path));       
                }
                else
                {
                    std::cout << "-16a file should be .png " << path << std::endl;
                    mode = helpMode;
                    break;
                }
            }
            else if (opt == "-16m")
            {
                i++;
                if (i == argc)
                {
                    std::cout << "-16m needs file path" << std::endl;
                    mode = helpMode;
                    break;
                }
                std::string path(argv[i]);
                if (check_ext(path, ".png"))
                {
                    outputs.push_back(output_type(MaskPNGFile, path));       
                }
                else
                {
                    std::cout << "-16m file should be .png " << path << std::endl;
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
            else
            {
                std::cout << "Unknow Parameter: " << opt << std::endl;
                mode = helpMode;
                break;
            }
        }
    }
}

void process_image(const char*& input_path, output_list& outputs, Mode& mode, FileType& inputType)
{
    Image* reader;
    bool hasAlphaChannel = false;

    if (inputType == PNGFile)
    {
        std::cout << "read png" << std::endl;
        reader = new PNGReader(input_path);
        hasAlphaChannel = (dynamic_cast<PNGReader*>(reader)->channels() == 4);
        std::cout << "finish" << std::endl;
    }
    else
    {
        reader = new JPEGReader(input_path);
    }

    if (reader->valid())
    {
        PNGWriter *mask_png_writer = 0;
        PNGWriter *alpha_png_writer = 0;
        PNGWriter *noalpha_png_writer = 0;
        PNGWriter *fullcolor_png_writer = 0;
        for (output_list::iterator i = outputs.begin(); i != outputs.end(); ++i)
        {
            FileType outputType = (*i).first;
            switch (outputType)
            {
            case MaskPNGFile:
                if (hasAlphaChannel)
                {
                    if (!mask_png_writer)
                    {
                        mask_png_writer = new PNGWriter(reader, hasAlphaChannel);
                        mask_png_writer->process(reduce_color<1>(reader, 5, 5, 5, (mode == previewMode)));
                    }
                    mask_png_writer->write((*i).second.c_str());
                }
                else
                {
                    if (!noalpha_png_writer)
                    {
                        noalpha_png_writer = new PNGWriter(reader, hasAlphaChannel);
                        noalpha_png_writer->process(reduce_color<0>(reader, 5, 6, 5, (mode == previewMode)));
                    }
                    noalpha_png_writer->write((*i).second.c_str());
                }
                break;
            case AlphaPNGFile:
                if (hasAlphaChannel)
                {
                    if (!alpha_png_writer)
                    {
                        alpha_png_writer = new PNGWriter(reader, hasAlphaChannel);
                        alpha_png_writer->process(reduce_color<4>(reader, 4, 4, 4, (mode == previewMode)));
                    }
                    alpha_png_writer->write((*i).second.c_str());
                }
                else
                {
                    if (!noalpha_png_writer)
                    {
                        noalpha_png_writer = new PNGWriter(reader, hasAlphaChannel);
                        noalpha_png_writer->process(reduce_color<0>(reader, 5, 6, 5, (mode == previewMode)));
                    }
                    noalpha_png_writer->write((*i).second.c_str());
                }
                break;
            case FullColorPNGFile:
                if (!fullcolor_png_writer)
                {
                    fullcolor_png_writer = new PNGWriter(reader, hasAlphaChannel);
                    std::cout << "start processing" << std::endl;
                    fullcolor_png_writer->process(reader->raw_image());
                }
                std::cout << "start writing" << std::endl;
                fullcolor_png_writer->write((*i).second.c_str());
                break;
            }
        }
        delete reader;
        if (mask_png_writer)
        {
            delete mask_png_writer;
        }
        if (alpha_png_writer)
        {
            delete alpha_png_writer;
        }
        if (noalpha_png_writer)
        {
            delete noalpha_png_writer;
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
    FileType inputType;

    parse_arg(argc, argv, input_path, outputs, mode, inputType);

    if (input_path == 0 || outputs.size() == 0)
    {
        mode = helpMode;
    }

    if (mode == helpMode)
    {
        help();
        return 1;
    }

    process_image(input_path, outputs, mode, inputType);

    return 0;
}
