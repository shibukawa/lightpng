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
    jpeg_decompress_struct jpeginfo_;
    jpeg_error_mgr jpegerr_;

    void destroy();
};

#endif
