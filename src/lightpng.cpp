#include <iostream>
#include <string>
#include <zlib.h>
#include <png.h>
#include "lightpng.h"
#include "PNGRead.h"
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


int main(int argc, const char** argv)
{
    const char* input_path = 0;
    const char* output_path = 0;
    Mode mode = textureMode; 
    ColorMode color = colorAutoDetect; 

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
            }
            else if (output_path == 0)
            {
                output_path = argv[i];
            }
            else
            {
                mode = helpMode;
                break;
            }
        }
    }
    if (output_path == 0)
    {
        mode = helpMode;
    }
    if (mode == helpMode)
    {
        help();
        return 1;
    }
    PNGRead reader(input_path);
    if (reader)
    {
        if (reader.channels() == 3)
        {
            switch (color)
            {
            case colorAutoDetect:
            case color4444:
                {
                    PNGWrite<0> writer(reader.width(), reader.height(), 5, 6, 5);
                    writer.process(reader.raw_image(), (mode == previewMode));
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
                {
                    PNGWrite<1> writer(reader.width(), reader.height(), 5, 5, 5);
                    writer.process(reader.raw_image(), (mode == previewMode));
                    writer.write(output_path);
                }
                break;
            case color4444:
                {
                    PNGWrite<4> writer(reader.width(), reader.height(), 4, 4, 4);
                    writer.process(reader.raw_image(), (mode == previewMode));
                    writer.write(output_path);
                }
                break;
            }
        }
    }
    return 0;
}
