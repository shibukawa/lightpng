#include <iostream>
#include <string>
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
              << std::endl
              << "  PNG file processing tool to use for texture." << std::endl
              << "  * It generetes small file size PNG file." << std::endl
              << "  * It reduce image color to fit 16 bit texture." << std::endl
              << std::endl
              << "  usage> lightpng [opt] input_image.png output_image.png" << std::endl
              << std::endl
              << "   -t, --texture : Texture Mode (default)" << std::endl
              << "   -p, --preview : Preview Mode" << std::endl
              << std::endl
              << "   -m, --mask    : RGBA 5551 or RGB 565 (default)" << std::endl
              << "   -a, --alpha   : RGBA 4444 (it is ignored if no alpha)" << std::endl
              << std::endl
              << "   -h, --help    : Show this message" << std::endl;
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


void parseArg(int argc, const char** argv, const char*& input_path, const char*& output_path, Mode& mode, ColorMode& color, InputFileType& inputType)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string opt(argv[i]);
        if (opt == "-h" || opt == "--help")
        {
            mode = helpMode;
            break;
        }
        else if (opt == "-p" || opt == "--preview")
        {
            mode = previewMode;
        }
        else if (opt == "-t" || opt == "--texture")
        {
            mode = textureMode;
        }
        else if (opt == "-m" || opt == "--mask")
        {
            color = colorAutoDetect;
        }
        else if (opt == "-a" || opt == "--alpha")
        {
            color = color4444;
        }
        else if (check_ext(opt, ".png"))
        {
            if (input_path == 0)
            {
                input_path = argv[i];
                inputType = PNGFile;
            }
            else if (output_path == 0)
            {
                output_path = argv[i];
            }
            else
            {
                std::cout << "Third file parameter is invalid" << std::endl;
                mode = helpMode;
                break;
            }
        }
        else if (check_ext(opt, ".jpg") || check_ext(opt, ".jpeg"))
        {
            if (input_path == 0)
            {
                input_path = argv[i];
                inputType = JPEGFile;
            }
            else
            {
                std::cout << "JPEG is acceptable only for input" << std::endl;
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

void processImage(const char*& input_path, const char*& output_path, Mode& mode, ColorMode& color, InputFileType& inputType)
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
        if (hasAlphaChannel)
        {
            switch (color)
            {
            case colorAutoDetect:
                {
                    PNGWrite<1> writer(reader->width(), reader->height(), 5, 5, 5);
                    writer.process(reader->raw_image(), (mode == previewMode));
                    writer.write(output_path);
                }
                break;
            case color4444:
                {
                    PNGWrite<4> writer(reader->width(), reader->height(), 4, 4, 4);
                    writer.process(reader->raw_image(), (mode == previewMode));
                    writer.write(output_path);
                }
                break;
            }
        }
        else
        {
            switch (color)
            {
            case colorAutoDetect:
            case color4444:
                {
                    PNGWrite<0> writer(reader->width(), reader->height(), 5, 6, 5);
                    writer.process(reader->raw_image(), (mode == previewMode));
                    writer.write(output_path);
                }
                break;
            }
        }
        delete reader;
    }
    else
    {
        std::cout << "Read error" << std::endl;
    }
}

int main(int argc, const char** argv)
{
    const char* input_path = 0;
    const char* output_path = 0;
    Mode mode = textureMode; 
    ColorMode color = colorAutoDetect;
    InputFileType inputType = NoneFile;

    parseArg(argc, argv, input_path, output_path, mode, color, inputType);

    if (output_path == 0)
    {
        mode = helpMode;
    }

    if (mode == helpMode)
    {
        help();
        return 1;
    }

    processImage(input_path, output_path, mode, color, inputType);

    return 0;
}
