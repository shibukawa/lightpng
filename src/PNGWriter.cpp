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
    Buffer() : totalsize_(0) {}
    ~Buffer() {
        std::vector<Chunk>::iterator i;
        for (i = chunks_.begin(); i != chunks_.end(); ++i)
        {
            delete[] (*i).buffer;
        }
    }

    size_t size() const
    {
        return totalsize_;
    }

    void write(png_bytep data, png_size_t length)
    {
        Chunk chunk;
        chunk.buffer = new png_byte[length];
        memcpy(chunk.buffer, data, length);
        chunk.size = length;
        chunks_.push_back(chunk);
        totalsize_ += length;
    }

    void flush(unsigned char*& destination, size_t& filesize) const
    {
        destination = new png_byte[totalsize_];
        size_t offset = 0;
        std::vector<Chunk>::const_iterator i;
        for (i = chunks_.begin(); i != chunks_.end(); ++i)
        {
            memcpy(destination + offset, (*i).buffer, (*i).size);
            offset += (*i).size;
        }
        filesize = totalsize_;
    }

private:
    std::vector<Chunk> chunks_;
    size_t totalsize_;
};

class MultithreadTask
{
public:
    MultithreadTask(PNGWriter* writer, size_t start_parameter, size_t end_parameter, bool verbose)
        : next_task_(start_parameter), last_task_(end_parameter), best_size_(1 << 31), best_index_(-1), writer_(writer),
          best_result_(0), verbose_(verbose)
    {
        pthread_mutex_init(&mutex_, NULL);
    }
    ~MultithreadTask()
    {
        pthread_mutex_destroy(&mutex_);
        if (best_result_)
        {
            delete best_result_;
        }
    }
    int get_next()
    {
        int result;
        pthread_mutex_lock(&mutex_);
        if (next_task_ < last_task_)
        {
            result = next_task_++;
        }
        else
        {
            result = -1;
        }
        pthread_mutex_unlock(&mutex_);
        return result;
    }
    void store_result(Buffer* buffer, size_t index)
    {
        pthread_mutex_lock(&mutex_);
        if (buffer->size() < best_size_)
        {
            best_size_ = buffer->size();
            best_index_ = index;
            if (best_result_)
            {
                delete best_result_;
            }
            best_result_ = buffer;
        }
        if (verbose_)
        {
            parameter* param = parameters + index;
            param->print(buffer->size());
        }
        pthread_mutex_unlock(&mutex_);
    }
    size_t best_size() const { return best_size_; }
    int best_index() const { return best_index_; }
    void flush(unsigned char*& file_content, size_t& filesize) const {
        if (best_result_)
        {
            best_result_->flush(file_content, filesize);
        }
    }
    static void* thread_main(void* param);

private:
    pthread_mutex_t mutex_;
    size_t next_task_;
    size_t last_task_;
    size_t best_size_;
    int best_index_;
    PNGWriter* writer_;
    Buffer* best_result_;
    bool verbose_;
};


void* MultithreadTask::thread_main(void* param)
{
    MultithreadTask* self = reinterpret_cast<MultithreadTask*>(param);
    int parameter_index = self->get_next();
    while (parameter_index != -1)
    {
        Buffer* buffer = new Buffer();
        self->writer_->compress(parameter_index, buffer);
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
    raw_buffer_ = raw_buffer;
    image_rows_.reset(new unsigned char*[height_]);
    size_t pixelSize = (has_alpha_) ? 4 : 3;
    for (size_t i = 0; i < height_; ++i)
    {
        image_rows_[i] = raw_buffer.get() + i * width_ * pixelSize;
    }
    process_();
}

bool PNGWriter::can_convert_index_color_(buffer_t raw_buffer)
{
    boost::unordered_map<unsigned int, unsigned char> colormap;
    buffer_t indexed_buffer(new unsigned char[width_ * height_]);
    if (has_alpha_)
    {
        unsigned char transcolor = 0;
        unsigned char opaquecolor = 255;
        for (size_t y = 0; y < height_; y++)
        {
            for (size_t x = 0; x < width_; x++)
            {
                size_t offset = (y * width_ + x) * 4;
                unsigned char alpha = raw_buffer[offset + 3];
                unsigned int colorCode = (alpha == 0) ? 0 : (raw_buffer[offset] << 24) + (raw_buffer[offset + 1] << 16) + (raw_buffer[offset + 2]<< 8) + alpha;
                boost::unordered_map<unsigned int, unsigned char>::iterator existing = colormap.find(colorCode);
                if (existing != colormap.end())
                {
                    indexed_buffer[y * width_ + x] = existing->second;
                }
                else
                {
                    if (alpha == 255)
                    {
                        indexed_buffer[y * width_ + x] = opaquecolor;
                        colormap[colorCode] = opaquecolor--;
                    }
                    else
                    {
                        indexed_buffer[y * width_ + x] = transcolor;
                        colormap[colorCode] = transcolor++;
                    }
                }
            }
        }
    }
    else
    {
        unsigned char color = 0;
        for (size_t y = 0; y < height_; y++)
        {
            for (size_t x = 0; x < width_; x++)
            {
                size_t offset = (y * width_ + x) * 3;
                unsigned int colorCode = (raw_buffer[offset] << 24) + (raw_buffer[offset + 1] << 16) + (raw_buffer[offset + 2]<< 8) + 255;
                boost::unordered_map<unsigned int, unsigned char>::iterator existing = colormap.find(colorCode);
                if (existing != colormap.end())
                {
                    indexed_buffer[y * width_ + x] = existing->second;
                }
                else
                {
                    indexed_buffer[y * width_ + x] = color;
                    colormap[colorCode] = color++;
                }
            }
        }
    }
    if (colormap.size() > 256)
    {
        if (verbose_)
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
    if (verbose_)
    {
        std::cout << "Export as index color png (auto convert: " << colormap.size() << " colors are used in this image)" << std::endl;
    }
    process(indexed_buffer, palette, trans, true);
    return true;
}

void PNGWriter::process(buffer_t raw_buffer, bool shrink)
{
    if (optimize_ == 0)
    {
        process(raw_buffer);
    }
    else if (!can_convert_index_color_(raw_buffer))
    {
        if (shrink)
        {
            buffer_t buffer(new unsigned char[width_ * height_ * 3]);
            Image::copy_4_to_3(width_, height_, raw_buffer, buffer);
            if (verbose_)
            {
                std::cout << "Export as 24 bit png(unused alpha channel was removed)" << std::endl;
            }
            process(buffer);
        }
        else if (has_alpha_)
        {
            buffer_t buffer(new unsigned char[width_ * height_ * 3]);
            bool clean = Image::copy_4_to_4(width_, height_, raw_buffer, buffer, true);
            if (clean && verbose_)
            {
                std::cout << "RGB space is cleand" << std::endl;
            }
            process(buffer);
        }
        else
        {
            process(raw_buffer);
        }
    }
}

void PNGWriter::process(buffer_t raw_buffer, palette_t palette, trans_t trans, bool palette_optimize)
{
    index_ = true;
    if (palette_optimize)
    {
        PaletteOptimizer optimizer(width_, height_);
        optimizer.process8bit(raw_buffer, palette, trans);
        raw_buffer_ = optimizer.buffer();
        palette_ = optimizer.palette();
        trans_ = optimizer.trans();
        palette_size_ = optimizer.palette_size();
        trans_size_ = optimizer.trans_size();
    }
    else
    {
        raw_buffer_ = raw_buffer;
        palette_ = palette;
        trans_ = trans;
        palette_size_ = 255;
        trans_size_ = 255;
    }
    image_rows_.reset(new unsigned char*[height_]);
    for (size_t i = 0; i < height_; ++i)
    {
        image_rows_[i] = raw_buffer_.get() + i * width_;
    }
    process_();
}

void PNGWriter::process_()
{
    Buffer* buffer;
    unsigned char* content;
    int best;

    switch (optimize_)
    {
    case 0:
        buffer = new Buffer();
        compress(6, buffer);
        content = file_content_.get();
        buffer->flush(content, file_size_);
        file_content_.reset(content);
        delete buffer;
        valid_ = true;
        break;
    case 1:
        optimizeWithOptions_(6, 30, true);
        break;
    case 2:
        best = optimizeWithOptions_(6, 12, false);
        if (best > 0)
        {
            optimizeWithOptions_(best - 6, best - 5, true);
        }
        break;
    case 3:
        optimizeWithOptions_(0, 30, true);
        break;
    }
}

int PNGWriter::optimizeWithOptions_(size_t start_parameter, size_t end_parameter, bool show_result)
{
    int rc;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    MultithreadTask task(this, start_parameter, end_parameter, verbose_);
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
        if (show_result && verbose_)
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
        unsigned char* content = file_content_.get();
        task.flush(content, file_size_);
        file_content_.reset(content);
        valid_ = true;
        return best_index;
    }
    return -1;
}

void PNGWriter::write(const char* filepath)
{
    if (valid_)
    {
        FILE * fp = fopen(filepath, "wb");
        fwrite(file_content_.get(), 1 , file_size_ , fp);
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
    if (index_)
    {
        size_t bitlength = 8;
        if (palette_size_ <= 2)
        {
            bitlength = 1;
            packing = true;
        }
        else if (palette_size_ <= 4)
        {
            bitlength = 2;
            packing = true;
        }
        else if (palette_size_ <= 16)
        {
            bitlength = 4;
            packing = true;
        }
        png_set_IHDR(png, info, width_, height_, bitlength, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
        png_set_PLTE(png, info, palette_.get(), palette_size_);
        if (trans_size_ > 0)
        {
            png_set_tRNS(png, info, trans_.get(), trans_size_, NULL);
        }
    }
    else if (has_alpha_)
    {
        png_set_IHDR(png, info, width_, height_, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    }
    else
    {
        png_set_IHDR(png, info, width_, height_, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    }
    png_write_info(png, info);
    if (packing)
    {
        png_set_packing(png);
    }
    png_write_image(png, image_rows_.get());
    png_write_end(png, info);
};
