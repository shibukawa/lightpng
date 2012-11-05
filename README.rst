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

ngCore already supports RGBA4444 and RGB565 format. You can create 16bit ``GL2.Texture``
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
   $ ./lightpng input.png -16m output.png

If you have MinGW environment, you can build Win32 binary on your environment ::

   $ sudo port install i386-mingw32-gcc scons
   $ soncs --mingw32
   $ wine lightpng.exe input.png -16m output.png

Usage
-----

Use this command like this::

   $ lightpng [opt] input_image [output_options]

``input_image`` can accept ``.png`` and ``.jpg`` file.

Options
~~~~~~~

:-b, --benchmark: Display time to process
:-v, --verbose: Display compression result
:-h, --help: Show this message

Output Options
~~~~~~~~~~~~~~

:-16m PATH: 16 bit PNG with 1 bit alpha (RGBA 5551). If source image doesn't have alpha, it generates RGB 565 PNG.
:-16a PATH: 16 bit PNG with 4 bit alpha (RGBA 4444). If source image doesn't have alpha, it generates RGB 565 PNG.
:-16 PATH: It is as same as ``-16a`` .
:-32 PATH: 24/32 bit PNG. It tries several compression option and reduce file size.
:-p16m PATH: Preview mode of ``-16m``.
:-p16a PATH: Preview mode of ``-16a``.
:-p16 PATH: Preview mode of ``-16``.

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

Non-Open Source Version
-----------------------

This program experimentally supports compressed texture generation (pvr/atc). It needs PVRTexLib and/or AdrenoSDK. They are available here:

* http://www.imgtec.com/powervr/insider/powervr-utilities.asp
* http://developer.qualcomm.com/develop/

AdrenoSDK is proveded in Windows execution. So you need wine to install and extract needed libraries/headers.

To create this version, set following flags to scons command:

:--no-opensource: Enable Texture Compression format support
:--PVRTexLib=DIR: Enable PVRTC Texture Compression convert/preview feature. Default is "./PVRTexLib".
:--AdrenoSDK=DIR: Enable ATITC Texture Compression convert/preview feature. Default is "~/.wine/drive_c/AdrenoSDK".

It adds following output options:

:-pvr PATH: 4 bpp PVRTC compressed texture file
:-lpvr PATH: 4 bpp PVRTC compressed texture file with legacy format (version 2)
:-ppvr PATH: Preview mode of PVRTC
:-atc PATH: 8 bpp ATITC compressed texture file
:-fatc PATH: 8 bpp ATITC compressed texture file with header information
:-patc PATH: Preview mode of PVRTC

If you use this option, use this program for internal use. You can't distribute your modified source code and/or binary in public.
If you want to reuse my source code to your product, please sent me Amazon.com or Amazon.co.jp e-mail gift to my address (yoshiki at shibu.jp).

I am reading the license of Ardeno SDK, but it maybe prohibit linking with open source tool (legal English is very difficult). If I find any good way to link with open source code, I will merge this code into main.

.. note::

   ``--no-opensource`` and ``--mingw32`` can not be enabled at the same time now.
   Please anyone tell me how to use closed source .lib via mingw32.

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

