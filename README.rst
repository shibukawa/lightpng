lightpng PNG optimization tool for game graphics
================================================ 

Purpose of This Tool
--------------------

PNG file processing tool to use for texture in game.

3D graphics API (OpenGL and DirectX) provides any efficiency texture formats
like RGBA5551, RGB565, RGBA444 and so on. They can save graphics' memory and
improve performance.

And current gaming API supports PNG file for texture. But PNG doesn't support
any 16 bit per pixel formats.

This program generates a color reducted PNG file with dithering and it uses
16 bit color space only. It generates high quality PNG files by using
`Floyd–Steinberg dithering <http://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering>`_
algorithm. So you can easy to create 16 bit texture from this file like it::


    // if it has no alpha channel
    png_byte  src[height][width * 3];
    uint16_t  texture[height * width];
    for (size_t y = 0; y < height; ++y)
    {
        for (size_t x = 0; x < width; ++x)
        {
            int r = src[y][x * 3];
            int g = src[y][x * 3];
            int b = src[y][x * 3];
            texture[y * width + x] = static_cast<uint16_t>((r << 11) | (g << 5) | (b));
        }
    }

PNG is east to handle for developers and designers. And the result PNG file become
smaller than the original PNG file. It will help many mobile game developers.

ngCore already supports RGBA5551 and RGB565 format. You can create 16bit ``GL2.Texture``
from this PNG file directly. 

How To Build
------------

It needs following libraries, but these source code are bundled:

* libpng
* libz
* jpeglib

This repository contains a build setting file of SCons. I tested on Mac OS X with MaccPorts environment,
but maybe work on any scons/gcc environment::

   $ sudo port install scons 
   $ scons
   $ ./lightpng input.png output.png

If you have MinGW environment, you can build Win32 binary on your environment ::

   $ sudo port install i386-mingw32-gcc scons
   $ soncs -f SConstruct.mingw32
   $ wine lightpng.exe input.png output.png

Usage
-----

Use this command like this::

   $ lightpng [opt] input_image output_image

``input_image`` can accept ``.png`` and ``.jpg`` file.
``output_image`` should be ``.png``.

Options
~~~~~~~

:-t, --texture: Texture Mode (default)
:-p, --preview: Preview Mode
:-m, --mask: RGBA 5551 or RGB 565 (default)
:-a, --alpha: RGBA 4444 (it is ignored if no alpha)
:-h, --help: Show this message

Texture Mode / Preview Mode
---------------------------

If you want to create final output, you should use texture mode.
But this file truncates unused bit, so it is not good for preview.

For example, RGBA 5551 format has only 1 bit for alpha channel. The solid color pixel's
alpha value become transparent.

**Before Convert**

.. list-table::

   - * R
     * ``1100 0000``
   - * G
     * ``1111 1111``
   - * B
     * ``0000 1111``
   - * A
     * ``1111 1111``

**After Convert**

.. list-table::

   - * R
     * ``1100 0000``
   - * G
     * ``1111 1000``
   - * B
     * ``0000 1000``
   - * A
     * ``1000 0000``

Preview mode completes the lack of this information. You can see final image on your favorite viewer.

License
-------

This source code is released under MIT License.

.. include:: LICENSE.rst

Author
------

:Copyright: Yoshiki Shibukawa (DeNA Co.,Ltd. / ngmoco:) LLC)
:Contact: yshibukawa at ngmoco.com

Thanks
------

This source code is created by other many people's help. Including my family, friends of Python/Sphinx communitiy, coworkers (DeNA, ngmoco)
and so on.

I learned good dithering algorithm from Sc4Freak's blog (http://adtsai.blogspot.com/2011/03/high-quality-dithering-for-windows.html) and
his source code. My implementation is an improvement/portable version of his code.

And thanks from Greg Roelofs (libpng author) and Jean-loup Gailly and Mark Adler (zlib authors), I can implement it in a day.

