#ifndef LIGHTPNG_H
#define LIGHTPNG_H

#include <vector>
#include <utility>
#include <string>

enum Mode
{
    helpMode,
    textureMode,
    previewMode
};

enum InputFileType
{
    JPEGFile,
    PNGFile
};

enum OutputFileType
{
    MaskPNGFile,
    AlphaPNGFile,
    FullColorPNGFile,
    PVRFile,
    LegacyPVRFile,
    ATCFile,
    ATCPlusHeaderFile
};

typedef std::pair<OutputFileType, std::string> output_type;
typedef std::vector<output_type> output_list;

#endif
