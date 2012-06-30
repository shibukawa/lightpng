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

enum FileType
{
    JPEGFile,
    PNGFile,
    MaskPNGFile,
    AlphaPNGFile,
    FullColorPNGFile,
    PVRFile,
    LegacyPVRFile,
    ATCFile,
    ATCPlusHeaderFile
};

typedef std::pair<FileType, std::string> output_type;
typedef std::vector<output_type> output_list;

#endif
