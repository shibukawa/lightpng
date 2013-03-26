#include <iostream>
#include <vector>
#include <stdio.h>
#include <boost/tr1/unordered_map.hpp>
#include <pthread.h>
#include <zlib.h>
#include <png.h>
#include "LPType.h"
#include "PNGWriter.h"
#include "PaletteOptimizer.h"


struct parameter
{
    int type;
    int strategy;
    int window_bits;
    int filter;

    void print(size_t size)
    {
        static const char* strategy_str[] = {
            "Z_DEFAULT_STRATEGY",
            "Z_FILTERED",
            "Z_HUFFMAN_ONLY",
            "Z_RLE",
            "Z_FIXED"
        };
        static const char* filter_str[] = {
            "PNG_FILTER_NONE",
            "PNG_FILTER_SUB",
            "PNG_FILTER_UP",
            "PNG_FILTER_AVG",
            "PNG_FILTER_PAETH",
            "PNG_ALL_FILTERS"
        };
        if (type == 0)
        {
            std::cout << size << " bytes (compressor: zLib, strategy: " << strategy_str[strategy] << ", filter: "
                      << filter_str[filter] << ")" << std::endl;
        }
        else
        {
            std::cout << size << " bytes (compressor: zopfli, filter: " << filter_str[filter] << ")" << std::endl;
        }
    }
    int get_type() const { return type; }
    int get_strategy() const { return strategy; }
    int get_window_bits() const { return window_bits; }
    int get_filter() const
    {
        static const int filters[] = {
            PNG_FILTER_NONE,
            PNG_FILTER_SUB,
            PNG_FILTER_UP,
            PNG_FILTER_AVG,
            PNG_FILTER_PAETH,
            PNG_ALL_FILTERS
        };
        return filters[filter];
    }
};

const int total_parameters = 36;

parameter parameters[total_parameters] = {
    { 1, 0, 15, 0 },
    { 1, 0, 15, 1 },
    { 1, 0, 15, 2 },
    { 1, 0, 15, 3 },
    { 1, 0, 15, 4 },
    { 1, 0, 15, 5 },
    { 0, 0, 15, 0 },
    { 0, 0, 15, 1 },
    { 0, 0, 15, 2 },
    { 0, 0, 15, 3 },
    { 0, 0, 15, 4 },
    { 0, 0, 15, 5 },
    { 0, 1, 15, 0 },
    { 0, 1, 15, 1 },
    { 0, 1, 15, 2 },
    { 0, 1, 15, 3 },
    { 0, 1, 15, 4 },
    { 0, 1, 15, 5 },
    { 0, 2, 15, 0 },
    { 0, 2, 15, 1 },
    { 0, 2, 15, 2 },
    { 0, 2, 15, 3 },
    { 0, 2, 15, 4 },
    { 0, 2, 15, 5 },
    { 0, 3, 15, 0 },
    { 0, 3, 15, 1 },
    { 0, 3, 15, 2 },
    { 0, 3, 15, 3 },
    { 0, 3, 15, 4 },
    { 0, 3, 15, 5 },
    { 0, 4, 15, 0 },
    { 0, 4, 15, 1 },
    { 0, 4, 15, 2 },
    { 0, 4, 15, 3 },
    { 0, 4, 15, 4 },
    { 0, 4, 15, 5 }
};

struct Chunk
{
    unsigned char* buffer;
    size_t size;
};

class Buffer
{
public:
    Buffer() : _totalsize(0) {}
    ~Buffer() {
        std::vector<Chunk>::iterator i;
        for (i = chunks_.begin(); i != chunks_.end(); ++i)
        {
            delete[] (*i).buffer;
        }
    }

    size_t size() const
    {
        return _totalsize;
    }

    void write(png_bytep data, png_size_t length)
    {
        Chunk chunk;
        chunk.buffer = new png_byte[length];
        memcpy(chunk.buffer, data, length);
        chunk.size = length;
        chunks_.push_back(chunk);
        _totalsize += length;
    }

    void flush(unsigned char*& destination, size_t& filesize) const
    {
        destination = new png_byte[_totalsize];
        size_t offset = 0;
        std::vector<Chunk>::const_iterator i;
        for (i = chunks_.begin(); i != chunks_.end(); ++i)
        {
            memcpy(destination + offset, (*i).buffer, (*i).size);
            offset += (*i).size;
        }
        filesize = _totalsize;
    }

private:
    std::vector<Chunk> chunks_;
    size_t _totalsize;
};

class MultithreadTask
{
public:
    MultithreadTask(PNGWriter* writer, size_t start_parameter, size_t end_parameter, bool verbose)
        : _next_task(start_parameter), _last_task(end_parameter), _best_size(1 << 31), _best_index(-1), _writer(writer),
          _best_result(0), _verbose(verbose)
    {
        pthread_mutex_init(&_mutex, NULL);
    }
    ~MultithreadTask()
    {
        pthread_mutex_destroy(&_mutex);
        if (_best_result)
        {
            delete _best_result;
        }
    }
    int get_next()
    {
        int result;
        pthread_mutex_lock(&_mutex);
        if (_next_task < _last_task)
        {
            result = _next_task++;
        }
        else
        {
            result = -1;
        }
        pthread_mutex_unlock(&_mutex);
        return result;
    }
    void store_result(Buffer* buffer, size_t index)
    {
        pthread_mutex_lock(&_mutex);
        if (buffer->size() < _best_size)
        {
            _best_size = buffer->size();
            _best_index = index;
            if (_best_result)
            {
                delete _best_result;
            }
            _best_result = buffer;
        }
        if (_verbose)
        {
            parameter* param = parameters + index;
            param->print(buffer->size());
        }
        pthread_mutex_unlock(&_mutex);
    }
    size_t best_size() const { return _best_size; }
    int best_index() const { return _best_index; }
    void flush(unsigned char*& file_content, size_t& filesize) const {
        if (_best_result)
        {
            _best_result->flush(file_content, filesize);
        }
    }
    static void* thread_main(void* param);

private:
    pthread_mutex_t _mutex;
    size_t _next_task;
    size_t _last_task;
    size_t _best_size;
    int _best_index;
    PNGWriter* _writer;
    Buffer* _best_result;
    bool _verbose;
};


void* MultithreadTask::thread_main(void* param)
{
    MultithreadTask* self = reinterpret_cast<MultithreadTask*>(param);
    int parameter_index = self->get_next();
    while (parameter_index != -1)
    {
        Buffer* buffer = new Buffer();
        self->_writer->compress(parameter_index, buffer);
        self->store_result(buffer, parameter_index);
        parameter_index = self->get_next();
    }
    pthread_exit(param);
}


void dummy_flash(png_structp png_ptr)
{
}


void buffer_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
    Buffer* buffer = reinterpret_cast<Buffer*>(png_get_io_ptr(png_ptr));
    buffer->write(data, length);
}


PNGWriter::~PNGWriter()
{
}

void PNGWriter::process(buffer_t raw_buffer)
{
    _raw_buffer = raw_buffer;
    _image_rows.reset(new unsigned char*[_height]);
    size_t pixelSize = (_has_alpha) ? 4 : 3;
    for (size_t i = 0; i < _height; ++i)
    {
        _image_rows[i] = raw_buffer.get() + i * _width * pixelSize;
    }
    _process();
}

bool PNGWriter::_can_convert_index_color(buffer_t raw_buffer)
{
    boost::unordered_map<unsigned int, unsigned char> colormap;
    buffer_t indexed_buffer(new unsigned char[_width * _height]);
    if (_has_alpha)
    {
        unsigned char transcolor = 0;
        unsigned char opaquecolor = 255;
        for (size_t y = 0; y < _height; y++)
        {
            for (size_t x = 0; x < _width; x++)
            {
                size_t offset = (y * _width + x) * 4;
                unsigned char alpha = raw_buffer[offset + 3];
                unsigned int colorCode = (alpha == 0) ? 0 : (raw_buffer[offset] << 24) + (raw_buffer[offset + 1] << 16) + (raw_buffer[offset + 2]<< 8) + alpha;
                boost::unordered_map<unsigned int, unsigned char>::iterator existing = colormap.find(colorCode);
                if (existing != colormap.end())
                {
                    indexed_buffer[y * _width + x] = existing->second;
                }
                else
                {
                    if (alpha == 255)
                    {
                        indexed_buffer[y * _width + x] = opaquecolor;
                        colormap[colorCode] = opaquecolor--;
                    }
                    else
                    {
                        indexed_buffer[y * _width + x] = transcolor;
                        colormap[colorCode] = transcolor++;
                    }
                }
            }
        }
    }
    else
    {
        unsigned char color = 0;
        for (size_t y = 0; y < _height; y++)
        {
            for (size_t x = 0; x < _width; x++)
            {
                size_t offset = (y * _width + x) * 3;
                unsigned int colorCode = (raw_buffer[offset] << 24) + (raw_buffer[offset + 1] << 16) + (raw_buffer[offset + 2]<< 8) + 255;
                boost::unordered_map<unsigned int, unsigned char>::iterator existing = colormap.find(colorCode);
                if (existing != colormap.end())
                {
                    indexed_buffer[y * _width + x] = existing->second;
                }
                else
                {
                    indexed_buffer[y * _width + x] = color;
                    colormap[colorCode] = color++;
                }
            }
        }
    }
    if (colormap.size() > 256)
    {
        if (_verbose)
        {
            std::cout << "Can't convert to index color png. It uses " << colormap.size() << " colors." << std::endl;
        }
        return false;
    }
    palette_t palette(new png_color[256]);
    trans_t trans(new unsigned char[256]);
    boost::unordered_map<unsigned int, unsigned char>::iterator iter;
    for (iter = colormap.begin(); iter != colormap.end(); iter++)
    {
        unsigned int colorCode = iter->first;
        palette[iter->second].red = (colorCode >> 24) & 0xff;
        palette[iter->second].green = (colorCode >> 16) & 0xff;
        palette[iter->second].blue = (colorCode >> 8) & 0xff;
        trans[iter->second] = colorCode & 0xff;
    }
    if (_verbose)
    {
        std::cout << "Export as index color png (auto convert: " << colormap.size() << " colors are used in this image)" << std::endl;
    }
    process(indexed_buffer, palette, trans, true);
    return true;
}

void PNGWriter::process(buffer_t raw_buffer, bool shrink)
{
    if (_optimize == 0)
    {
        process(raw_buffer);
    }
    else if (!_can_convert_index_color(raw_buffer))
    {
        if (shrink)
        {
            buffer_t buffer(new unsigned char[_width * _height * 3]);
            for (size_t y = 0; y < _height; y++)
            {
                for (size_t x = 0; x < _width; x++)
                {
                    size_t offset1 = (y * _width + x) * 3;
                    size_t offset2 = (y * _width + x) * 4;
                    buffer[offset1]     = raw_buffer[offset2];
           	        buffer[offset1 + 1] = raw_buffer[offset2 + 1];
                    buffer[offset1 + 2] = raw_buffer[offset2 + 2];
                }
            }
            if (_verbose)
            {
                std::cout << "Export as 24 bit png(unused alpha channel was removed)" << std::endl;
            }
            process(buffer);
        }
        else if (_has_alpha)
        {
            bool clean = false;
            for (size_t y = 0; y < _height; y++)
            {
                for (size_t x = 0; x < _width; x++)
                {
                    size_t offset = (y * _width + x) * 4;
                    if (raw_buffer[offset + 3] == 0)
                    {
                        clean = clean || (raw_buffer[offset] != 0) || (raw_buffer[offset + 1] != 0) || (raw_buffer[offset + 2] != 0);
                        raw_buffer[offset] = 0;
                        raw_buffer[offset + 1] = 0;
                        raw_buffer[offset + 2] = 0;
                    }
                }
            }
            if (clean && _verbose)
            {
                std::cout << "RGB space is cleand" << std::endl;
            }
            process(raw_buffer);
        }
        else
        {
            process(raw_buffer);
        }
    }
}

void PNGWriter::process(buffer_t raw_buffer, palette_t palette, trans_t trans, bool palette_optimize)
{
    _index = true;
    if (palette_optimize)
    {
        PaletteOptimizer optimizer(_width, _height);
        optimizer.process8bit(raw_buffer, palette, trans);
        _raw_buffer = optimizer.buffer();
        _palette = optimizer.palette();
        _trans = optimizer.trans();
        _palette_size = optimizer.palette_size();
        _trans_size = optimizer.trans_size();
    }
    else
    {
        _raw_buffer = raw_buffer;
        _palette = palette;
        _trans = trans;
        _palette_size = 255;
        _trans_size = 255;
    }
    _image_rows.reset(new unsigned char*[_height]);
    for (size_t i = 0; i < _height; ++i)
    {
        _image_rows[i] = _raw_buffer.get() + i * _width;
    }
    _process();
}

void PNGWriter::_process()
{
    Buffer* buffer;
    unsigned char* content;
    int best;

    switch (_optimize)
    {
    case 0:
        buffer = new Buffer();
        compress(6, buffer);
        content = _file_content.get();
        buffer->flush(content, _file_size);
        _file_content.reset(content);
        delete buffer;
        _valid = true;
        break;
    case 1:
        _optimizeWithOptions(6, 30, true);
        break;
    case 2:
        best = _optimizeWithOptions(6, 12, false);
        if (best > 0)
        {
            _optimizeWithOptions(best - 6, best - 5, true);
        }
        break;
    case 3:
        _optimizeWithOptions(0, 30, true);
        break;
    }
}

int PNGWriter::_optimizeWithOptions(size_t start_parameter, size_t end_parameter, bool show_result)
{
    int rc;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    MultithreadTask task(this, start_parameter, end_parameter, _verbose);
    size_t threadCount = std::min(static_cast<size_t>(8), end_parameter - start_parameter);
    pthread_t threads[8];
    for (size_t i = 0; i < threadCount; ++i)
    {
        rc = pthread_create(&threads[i], &attr, MultithreadTask::thread_main, reinterpret_cast<void*>(&task));
        if (rc)
        {
            std::cerr << "pthread create error" << std::endl;
        }
    }
    for (size_t i = 0; i < threadCount; ++i)
    {
        void* status;
        rc = pthread_join(threads[i], &status);
        if (rc)
        {
            std::cerr << "pthread join error" << std::endl;
        }
    }
    int best_index = task.best_index();
    if (best_index > -1)
    {
        if (show_result && _verbose)
        {
            if (threadCount == 1)
            {
                std::cout << "Result: ";
            }
            else
            {
                std::cout << "Best Result: ";
            }
            parameters[best_index].print(task.best_size());
        }
        unsigned char* content = _file_content.get();
        task.flush(content, _file_size);
        _file_content.reset(content);
        _valid = true;
        return best_index;
    }
    return -1;
}

void PNGWriter::write(const char* filepath)
{
    if (_valid)
    {
        FILE * fp = fopen(filepath, "wb");
        fwrite(_file_content.get(), 1 , _file_size , fp);
        fclose(fp);
    }
}

void PNGWriter::compress(size_t parameter_index, Buffer* buffer)
{
    png_structp png = 0;
    png_infop info = 0;
    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (!png)
    {
        return;
    }
    info = png_create_info_struct(png);
    if (!info)
    {
        return;
    }

    png_set_write_fn(png, reinterpret_cast<void*>(buffer), buffer_write, dummy_flash);

    parameter* param = parameters + parameter_index;
    if (param->get_type() == PNG_WRITER_USE_ZLIB)
    {
        png_set_filter(png, PNG_FILTER_TYPE_BASE, param->get_filter());
        png_set_compressor_type(png, PNG_WRITER_USE_ZLIB);
        png_set_compression_strategy(png, param->get_strategy());
        png_set_compression_window_bits(png, param->get_window_bits());
        png_set_compression_level(png, Z_BEST_COMPRESSION);
        png_set_compression_mem_level(png, MAX_MEM_LEVEL);
    }
    else // PNG_WRITER_USE_ZOPFLI
    {
        png_set_filter(png, PNG_FILTER_TYPE_BASE, param->get_filter());
        png_set_compressor_type(png, PNG_WRITER_USE_ZOPFLI);
        png_set_compression_level(png, 15);
    }
    bool packing = false;
    if (_index)
    {
        size_t bitlength = 8;
        if (_palette_size <= 2)
        {
            bitlength = 1;
            packing = true;
        }
        else if (_palette_size <= 4)
        {
            bitlength = 2;
            packing = true;
        }
        else if (_palette_size <= 16)
        {
            bitlength = 4;
            packing = true;
        }
        png_set_IHDR(png, info, _width, _height, bitlength, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        png_set_PLTE(png, info, _palette.get(), _palette_size);
        if (_trans_size > 0)
        {
            png_set_tRNS(png, info, _trans.get(), _trans_size, NULL);
        }
    }
    else if (_has_alpha)
    {
        png_set_IHDR(png, info, _width, _height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    }
    else
    {
        png_set_IHDR(png, info, _width, _height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    }
    png_write_info(png, info);
    if (packing)
    {
        png_set_packing(png);
    }
    png_write_image(png, _image_rows.get());
    png_write_end(png, info);
};
