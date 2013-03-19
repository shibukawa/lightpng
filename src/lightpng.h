#ifndef LIGHTPNG_H
#define LIGHTPNG_H

#include <vector>
#include <utility>
#include <string>

enum Mode
{
    helpMode,
    textureMode
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
    IndexedColorPNGFile,
    IndexedReducedColorPNGFile,
    PVRFile,
    LegacyPVRFile,
    ATCFile,
    ATCPlusHeaderFile,
    PreviewMaskPNGFile,
    PreviewAlphaPNGFile,
    PreviewPVRFile,
    PreviewATCFile
};

typedef std::pair<OutputFileType, std::string> output_type;
typedef std::vector<output_type> output_list;

#endif
