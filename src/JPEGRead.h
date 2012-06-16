#ifndef JPEGREAD_H
#define JPEGREAD_H

#include <jpeglib.h>
#include "lightpng.h"
#include "Read.h"

class JPEGRead : public Read
{
public:
    explicit JPEGRead(const char* filepath);
    ~JPEGRead();

private:
    jpeg_decompress_struct _jpeginfo;
    jpeg_error_mgr _jpegerr;

    void destroy();
};

#endif
