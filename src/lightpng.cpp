#include <iostream>
#include <zlib.h>
#include <png.h>
#include "lightpng.h"
#include "Read.h"
#include "PNGRead.h"
#include "JPEGRead.h"
#include "PNGWrite.h"

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


void parseArg(int argc, const char** argv, const char*& input, output_list& outputs, Mode& mode, FileType& inputType)
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
            else
            {
                std::cout << "Unknow Parameter: " << opt << std::endl;
                mode = helpMode;
                break;
            }
        }
    }
}

void processImage(const char*& input_path, output_list& outputs, Mode& mode, FileType& inputType)
{
    Read* reader;
    bool hasAlphaChannel = false;

    if (inputType == PNGFile)
    {
        reader = new PNGRead(input_path);
        hasAlphaChannel = (dynamic_cast<PNGRead*>(reader)->channels() == 4);
    }
    else
    {
        reader = new JPEGRead(input_path);
    }

    if (reader->valid())
    {
        PNGWrite<1> *mask_png_writer = 0;
        PNGWrite<4> *alpha_png_writer = 0;
        PNGWrite<0> *noalpha_png_writer = 0;
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
                        mask_png_writer = new PNGWrite<1>(reader->width(), reader->height(), 5, 5, 5);
                        mask_png_writer->process(reader->raw_image(), (mode == previewMode));
                    }
                    mask_png_writer->write((*i).second.c_str());
                }
                else
                {
                    if (!noalpha_png_writer)
                    {
                        noalpha_png_writer = new PNGWrite<0>(reader->width(), reader->height(), 5, 6, 5);
                        noalpha_png_writer->process(reader->raw_image(), (mode == previewMode));
                    }
                    noalpha_png_writer->write((*i).second.c_str());
                }
                break;
            case AlphaPNGFile:
                if (!alpha_png_writer)
                {
                    alpha_png_writer = new PNGWrite<4>(reader->width(), reader->height(), 4, 4, 4);
                    alpha_png_writer->process(reader->raw_image(), (mode == previewMode));
                }
                alpha_png_writer->write((*i).second.c_str());
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

    parseArg(argc, argv, input_path, outputs, mode, inputType);

    if (input_path == 0 || outputs.size() == 0)
    {
        mode = helpMode;
    }

    if (mode == helpMode)
    {
        help();
        return 1;
    }

    processImage(input_path, outputs, mode, inputType);

    return 0;
}
