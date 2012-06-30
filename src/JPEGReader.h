#ifndef JPEGREADER_H
#define JPEGREADER_H

#include <jpeglib.h>
#include "lightpng.h"
#include "Image.h"

class JPEGReader : public Image
{
public:
    explicit JPEGReader(const char* filepath);
    ~JPEGReader();

private:
    jpeg_decompress_struct _jpeginfo;
    jpeg_error_mgr _jpegerr;

    void destroy();
};

#endif
