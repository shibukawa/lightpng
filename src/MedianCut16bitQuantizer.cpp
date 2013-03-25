#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include "MedianCut16bitQuantizer.h"

// This source code uses Median cut algorithm C++.
// This is an original license.

/* Copyright (c) 2013 the authors listed at the following URL, and/or
the authors of referenced articles or incorporated external code:
http://en.literateprograms.org/Median_cut_algorithm_(C_Plus_Plus)?action=history&offset=20080309133934

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from: http://en.literateprograms.org/Median_cut_algorithm_(C_Plus_Plus)?oldid=12754
*/

Block::Block(Point* points, int pointsLength)
{
    this->_points = points;
    this->_pointsLength = pointsLength;
    for(int i=0; i < NUM_DIMENSIONS; i++)
    {
        _minCorner.x[i] = std::numeric_limits<unsigned char>::min();
        _maxCorner.x[i] = std::numeric_limits<unsigned char>::max();
    }
    _colorIndex = -1;
}

int Block::calcLongestSide(int R, int G, int B, int A)
{
    int m = -1;
    int thresholds[4] = {R, G, B, A};
    int maxIndex = 0;
    int loop = (A == 256) ? NUM_DIMENSIONS - 1 : NUM_DIMENSIONS;
    for(int i=0; i < loop; i++)
    {
  		int maxNum = _maxCorner.x[i];
   		int minNum = _minCorner.x[i];
        int diff = maxNum - minNum;
   		int maxArea = maxNum / thresholds[i];
   		int minArea = minNum / thresholds[i];
   		if (maxArea == minArea)
   		{
   			diff = 0;
    	}
        if (diff > m)
        {
            m = diff;
            maxIndex = i;
        }
    }
    _longestSideIndex = maxIndex;
    _longestSideLength = m;
}
bool Block::operator<(const Block& rhs) const
{
    return this->longestSideLength() < rhs.longestSideLength();
}
void Block::shrink()
{
    int i,j;
    for(j=0; j<NUM_DIMENSIONS; j++)
    {
        _minCorner.x[j] = _maxCorner.x[j] = _points[0].x[j];
    }
    for(i=1; i < _pointsLength; i++)
    {
        for(j=0; j<NUM_DIMENSIONS; j++)
        {
            _minCorner.x[j] = min(_minCorner.x[j], _points[i].x[j]);
            _maxCorner.x[j] = max(_maxCorner.x[j], _points[i].x[j]);
        }
    }
}

inline size_t calcColorCode(int r, int g, int b, int a, int R, int G, int B, int A)
{
    r = std::min(255, r);
    g = std::min(255, g);
    b = std::min(255, b);
    a = std::min(255, a);
	return ((r >> (8 - R)) << (G + B + A)) + ((g >> (8 - G)) << (B + A)) + ((b >> (8 - B)) << A) + (a >> (8 - A));
}

void Block::setColorIndex(int R, int G, int B, int A, size_t colorIndex, boost::scoped_array<short>& colorMap)
{
	size_t stepR = 1 << (8 - R);
	size_t stepG = 1 << (8 - G);
	size_t stepB = 1 << (8 - B);
	size_t minR = _minCorner.x[0];
	size_t minG = _minCorner.x[1];
	size_t minB = _minCorner.x[2];
	size_t maxR = _maxCorner.x[0] + stepR;
	size_t maxG = _maxCorner.x[1] + stepG;
	size_t maxB = _maxCorner.x[2] + stepB;
	if (A == 0)
	{
		for (int r = minR; r < maxR; r += stepR)
		{
			for (int g = minG; g < maxG; g += stepG)
			{
				for (int b = minB; b < maxB; b += stepB)
				{
					size_t colorCode = calcColorCode(r, g, b, 0, R, G, B, 0);
					colorMap[colorCode] = colorIndex;
				}
			}
		}
	}
	else
	{
		size_t stepA = 1 << (8 - A);
		size_t minA = _minCorner.x[3];
		size_t maxA = _maxCorner.x[3] + stepA;
		for (int r = minR; r < maxR; r += stepR)
		{
			for (int g = minG; g < maxG; g += stepG)
			{
				for (int b = minB; b < maxB; b += stepB)
				{
					for (int a = minA; a < maxA; a += stepA)
					{
						size_t colorCode = calcColorCode(r, g, b, a, R, G, B, A);
						colorMap[colorCode] = colorIndex;
					}
				}
			}
		}
	}
	_colorIndex = colorIndex;
}

void Block::calcAverageColor(unsigned char& new_r, unsigned char& new_g, unsigned char& new_b, unsigned char& new_a)
{
    double sum[NUM_DIMENSIONS] = {0, 0, 0, 0};
    for(int i=0; i < _pointsLength; i++)
    {
        for(int j=0; j < NUM_DIMENSIONS; j++)
        {
        	int value = (int)_points[i].x[j];
            sum[j] = sum[j] + value;
        }
    }
    new_r = sum[0] / _pointsLength;
    new_g = sum[1] / _pointsLength;
    new_b = sum[2] / _pointsLength;
    new_a = sum[3] / _pointsLength;
}

class CompareBlock {
public:
    bool operator()(boost::shared_ptr<Block>& b1, boost::shared_ptr<Block>& b2)
    {
    	return b1->longestSideLength() < b2->longestSideLength();
    }
};

template <int index>
class CoordinatePointComparator
{
public:
    bool operator()(Point left, Point right)
    {
        return left.x[index] < right.x[index];
    }
};

short MedianCut16bitQuantizer::searchNearestColor(int r, int g, int b, int a, bool skipAlpha)
{
	double minDistance = 256 * 256 * 5;
	short index = -1;
	std::vector<boost::shared_ptr<Block> >::iterator i;
	if (skipAlpha)
	{
		for (i = _blocks.begin(); i != _blocks.end(); i++)
		{
			Point* min = (*i)->minCorner();
			Point* max = (*i)->maxCorner();
			int rd = (r < min->x[0]) ? (min->x[0] - r) : ((max->x[0] < r) ? (r - max->x[0]) : 0);
			int gd = (g < min->x[1]) ? (min->x[1] - g) : ((max->x[1] < g) ? (g - max->x[1]) : 0);
			int bd = (b < min->x[2]) ? (min->x[2] - b) : ((max->x[2] < b) ? (b - max->x[2]) : 0);
			int distance = rd * rd + gd * gd + bd * bd;
			if (distance < minDistance)
			{
				minDistance = distance;
				index = (*i)->colorIndex();
			}
		}
	}
	else
	{
		for (i = _blocks.begin(); i != _blocks.end(); i++)
		{
			Point* min = (*i)->minCorner();
			Point* max = (*i)->maxCorner();
			int rd = (r < min->x[0]) ? (min->x[0] - r) : ((max->x[0] < r) ? (r - max->x[0]) : 0);
			int gd = (g < min->x[1]) ? (min->x[1] - g) : ((max->x[1] < g) ? (g - max->x[1]) : 0);
			int bd = (b < min->x[2]) ? (min->x[2] - b) : ((max->x[2] < b) ? (b - max->x[2]) : 0);
			int ad = (a < min->x[3]) ? (min->x[3] - a) : ((max->x[3] < a) ? (a - max->x[3]) : 0);
			int distance = rd * rd + gd * gd + bd * bd + ad * ad;
			if (distance < minDistance)
			{
				minDistance = distance;
				index = (*i)->colorIndex();
			}
		}
	}
	return index;
}


void MedianCut16bitQuantizer::quantize(size_t R, size_t G, size_t B, size_t A)
{
	unsigned int desiredSize = 256;
	boost::scoped_array<unsigned char> points(new unsigned char[4 * _width * _height]);
    memcpy(points.get(), _rawsrc.get(), _height * _width * 4);

    Point* image = reinterpret_cast<Point*>(points.get());

    size_t numPoints = _width * _height;

    const int ThresholdR = 1 << (8 - R);
    const int ThresholdG = 1 << (8 - G);
    const int ThresholdB = 1 << (8 - B);
    const int ThresholdA = 1 << (8 - A);
    const unsigned int filterR = (1 << 8) - ThresholdR;
    const unsigned int filterG = (1 << 8) - ThresholdG;
    const unsigned int filterB = (1 << 8) - ThresholdB;
    const unsigned int filterA = (1 << 8) - ThresholdA;

    std::priority_queue<boost::shared_ptr<Block>, std::vector<boost::shared_ptr<Block> >, CompareBlock> blockQueue;
    _tree.reset(new Block(image, numPoints));
    _tree->shrink();
	_tree->calcLongestSide(ThresholdR, ThresholdG, ThresholdB, ThresholdA);
    blockQueue.push(_tree);

    while (blockQueue.size() < desiredSize && blockQueue.top()->numPoints() > 1)
    {
        boost::shared_ptr<Block> longestBlock(blockQueue.top());

        blockQueue.pop();
        Point* begin  = longestBlock->getPoints();
		Point* median = longestBlock->getPoints() + (longestBlock->numPoints()+1)/2;
		Point* end    = longestBlock->getPoints() + longestBlock->numPoints();
		size_t longestSideIndex = longestBlock->longestSideIndex();
		switch(longestSideIndex)
		{
		case 0:
			std::nth_element(begin, median, end, CoordinatePointComparator<0>());
			break;
		case 1:
			std::nth_element(begin, median, end, CoordinatePointComparator<1>());
			break;
		case 2:
			std::nth_element(begin, median, end, CoordinatePointComparator<2>());
			break;
		case 3:
			std::nth_element(begin, median, end, CoordinatePointComparator<3>());
			break;
		}

		boost::shared_ptr<Block> block1(new Block(begin, median-begin));
		boost::shared_ptr<Block> block2(new Block(median, end-median));
		block1->shrink();
		block2->shrink();
		block1->calcLongestSide(ThresholdR, ThresholdG, ThresholdB, ThresholdA);
		block2->calcLongestSide(ThresholdR, ThresholdG, ThresholdB, ThresholdA);
	    blockQueue.push(block1);
	    blockQueue.push(block2);
    }

    const int OriginalMax = (1 << 8) - 1;

    size_t transindex = 0;
    size_t opaqueindex = 255;
    boost::scoped_array<short> colorMap(new short[65536]);
    for (size_t i = 0; i < 65536; i++)
    {
    	colorMap[i] = -1;
    }

    while (!blockQueue.empty())
    {
        boost::shared_ptr<Block> block = blockQueue.top();
        _blocks.push_back(block);
        blockQueue.pop();
        unsigned char new_r, new_g, new_b, new_a;
        block->calcAverageColor(new_r, new_g, new_b, new_a);

        new_r &= filterR;
        new_g &= filterG;
        new_b &= filterB;
        size_t index;
        if (A == 0)
        {
        	new_a = 255;
        	index = opaqueindex--;
        }
        else
        {
        	new_a &= filterA;
        	if (new_a == filterA)
        	{
	        	index = opaqueindex--;
        	}
        	else
        	{
	        	index = transindex++;
        	}
        }
        _palette[index].red   = new_r;
        _palette[index].green = new_g;
       	_palette[index].blue  = new_b;
        _trans[index]         = new_a;
        block->setColorIndex(R, G, B, A, index, colorMap);
    }

    for (size_t y = 0; y < _height; y++)
    {
    	for (size_t x = 0; x < _width; x++)
    	{
            int old_r, old_g, old_b, old_a;
            get(_rawsrc, x, y, old_r, old_g, old_b, old_a);
        

            size_t colorCode = calcColorCode(old_r, old_g, old_b, old_a, R, G, B, A);
        	short paletteIndex = colorMap[colorCode];

        	if (paletteIndex = -1)
        	{
        		paletteIndex = searchNearestColor(old_r, old_g, old_b, old_a, A == 0);
        		colorMap[colorCode] = paletteIndex;
        	}

            unsigned char new_r = _palette[paletteIndex].red;
            unsigned char new_g = _palette[paletteIndex].green;
            unsigned char new_b = _palette[paletteIndex].blue;
            unsigned char new_a = _trans[paletteIndex];

            set(_rawdest, x, y, paletteIndex);

            int dr = std::abs(old_r - new_r);
            int dg = std::abs(old_g - new_g);
            int db = std::abs(old_b - new_b);
            int da = std::abs(old_a - new_a);

            int r, g, b, a;
            get(_rawsrc, x + 1, y, r, g, b, a);
            r += dr * 7 / 16;
            g += dg * 7 / 16;
            b += db * 7 / 16;
            a += da * 7 / 16;
            set(_rawsrc, x + 1, y, r, g, b, a);

            get(_rawsrc, x - 1, y + 1, r, g, b, a);
            r += dr * 3 / 16;
            g += dg * 3 / 16;
            b += db * 3 / 16;
            a += da * 3 / 16;
            set(_rawsrc, x - 1, y + 1, r, g, b, a);

            get(_rawsrc, x, y + 1, r, g, b, a);
            r += dr * 5 / 16;
            g += dg * 5 / 16;
            b += db * 5 / 16;
            a += da * 5 / 16;
            set(_rawsrc, x, y + 1, r, g, b, a);

            get(_rawsrc, x + 1, y + 1, r, g, b, a);
            r += dr * 1 / 16;
            g += dg * 1 / 16;
            b += db * 1 / 16;
            a += da * 1 / 16;
            set(_rawsrc, x + 1, y + 1, r, g, b, a);
    	}
    }
}


void MedianCut16bitQuantizer::fixPalette(size_t R, size_t G, size_t B, size_t A)
{
    const int ThresholdR = 1 << (8 - R);
    const int ThresholdG = 1 << (8 - G);
    const int ThresholdB = 1 << (8 - B);
    const unsigned int filterR = (1 << 8) - ThresholdR;
    const unsigned int filterG = (1 << 8) - ThresholdG;
    const unsigned int filterB = (1 << 8) - ThresholdB;
    if (A == 0)
    {
    	for (size_t i = 0; i < 256; i++)
		{
        	_palette[i].red   = (int)_palette[i].red * 255 / filterR;
        	_palette[i].green = (int)_palette[i].green * 255 / filterG;
       		_palette[i].blue  = (int)_palette[i].blue * 255 / filterB;
        }
    }
    else
    {
	    const int ThresholdA = 1 << (8 - A);
	    const unsigned int filterA = (1 << 8) - ThresholdA;
    	for (size_t i = 0; i < 256; i++)
		{
        	_palette[i].red   = (int)_palette[i].red * 255 / filterR;
        	_palette[i].green = (int)_palette[i].green * 255 / filterG;
       		_palette[i].blue  = (int)_palette[i].blue * 255 / filterB;
        	_trans[i]         = (int)_trans[i] * 255 / filterA;
        }
    }
}

void median_cut_16bit_quantize(Image& image, PNGWriter& writer, bool hasAlphaChannel, bool hasAlpha, bool preview)
{
    MedianCut16bitQuantizer quantizer(image.width(), image.height());
    quantizer.process(image.image(), hasAlphaChannel);
    if (hasAlpha)
    {
	    quantizer.quantize(4, 4, 4, 4);
	    if (preview)
	    {
	    	quantizer.fixPalette(4, 4, 4, 4);
	    }
    }
    else
    {
	    quantizer.quantize(5, 6, 5, 0);
	    if (preview)
	    {
	    	quantizer.fixPalette(5, 6, 5, 0);
	    }
    }
    writer.process(quantizer.buffer(), quantizer.palette(), quantizer.trans(), true);
}
