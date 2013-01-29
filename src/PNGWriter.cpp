#include "PNGWriter.h"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <zlib.h>
#include <png.h>

struct parameter
{
    int strategy;
    int window_bits;
    int filter;

    void print(size_t size)
    {
        static const char* strategy_str[] = {
            "Z_DEFAULT_STRATEGY",
            "Z_FILTERED",
            "Z_HUFFMAN_ONLY",
            "Z_RLE"
        };
        static const char* filter_str[] = {
            "PNG_FILTER_NONE",
            "PNG_FILTER_SUB",
            "PNG_FILTER_UP",
            "PNG_FILTER_AVG",
            "PNG_FILTER_PAETH",
            "PNG_ALL_FILTERS"
        };
        std::cout << size << " bytes (strategy: " << strategy_str[strategy] << " filter: "
                  << filter_str[filter] << std::endl;
    }
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

parameter parameters[24] = {
    { 0, 15, 0 },
    { 0, 15, 1 },
    { 0, 15, 2 },
    { 0, 15, 3 },
    { 0, 15, 4 },
    { 0, 15, 5 },
    { 1, 15, 0 },
    { 1, 15, 1 },
    { 1, 15, 2 },
    { 1, 15, 3 },
    { 1, 15, 4 },
    { 1, 15, 5 },
    { 2,  9, 0 },
    { 2,  9, 1 },
    { 2,  9, 2 },
    { 2,  9, 3 },
    { 2,  9, 4 },
    { 2,  9, 5 },
    { 3,  9, 0 },
    { 3,  9, 1 },
    { 3,  9, 2 },
    { 3,  9, 3 },
    { 3,  9, 4 },
    { 3,  9, 5 }
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
    MultithreadTask(PNGWriter* writer, bool verbose)
        : _next_task(0), _best_size(1 << 31), _best_index(-1), _writer(writer),
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
        if (_next_task < 24)
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
    int _next_task;
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
    if (_raw_buffer)
    {
        delete[] _raw_buffer;
        delete[] _image_rows;
    }
    if (_file_content)
    {
        delete[] _file_content;
    }
}

void PNGWriter::process(unsigned char* raw_buffer)
{
    _raw_buffer = raw_buffer;
    _image_rows = new unsigned char*[_height];
    size_t pixelSize = (_has_alpha) ? 4 : 3;
    for (size_t i = 0; i < _height; ++i)
    {
        _image_rows[i] = raw_buffer + i * _width * pixelSize;
    }
    process(_image_rows);
}

void PNGWriter::process(unsigned char** image_rows)
{
    _image_rows = image_rows;

    if (_optimize)
    {
        int rc;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        MultithreadTask task(this, _verbose);
        pthread_t threads[8];
        for (size_t i = 0; i < 8; ++i)
        {
            rc = pthread_create(&threads[i], &attr, MultithreadTask::thread_main, reinterpret_cast<void*>(&task));
            if (rc)
            {
                std::cout << "pthread create error" << std::endl;
            }
        }
        for (size_t i = 0; i < 8; ++i)
        {
            void* status;
            rc = pthread_join(threads[i], &status);
            if (rc)
            {
                std::cout << "pthread join error" << std::endl;
            }
        }
        int best_index = task.best_index();
        if (best_index > -1)
        {
            if (_verbose)
            {
                std::cout << "best result: ";
                parameters[best_index].print(task.best_size());
            }
            task.flush(_file_content, _file_size);
            _valid = true;
        }
    }
    else
    {
        Buffer* buffer = new Buffer();
        compress(0, buffer);
        buffer->flush(_file_content, _file_size);
        delete buffer;
        _valid = true;
    }
}

void PNGWriter::write(const char* filepath)
{
    if (_valid)
    {
        FILE * fp = fopen(filepath, "wb");
        fwrite(_file_content , 1 , _file_size , fp);
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
    png_set_compression_strategy(png, param->get_strategy());
    png_set_compression_window_bits(png, param->get_window_bits());
    png_set_filter(png, PNG_FILTER_TYPE_BASE, param->get_filter());
    png_set_compression_level(png, Z_BEST_COMPRESSION);
    png_set_compression_mem_level(png, MAX_MEM_LEVEL);
    if (_has_alpha)
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
    png_write_image(png, _image_rows);
    png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
    png_write_end(png, info);
};
