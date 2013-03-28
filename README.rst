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

Why the File become Smaller? / What does this tool do?
------------------------------------------------------

.. image:: http://farm9.staticflickr.com/8367/8596428211_c454143237_n.jpg
   :target: http://www.flickr.com/photos/shibukawa/8596428211/

* 16 Bit Conversion:

  Cut lower bit.

* Dithering:

  Standard Floyd-Steinberg dithering.

* Quantize(1):

  It uses `ligimagequant <http://pngquant.org/lib/>`_.

* Quantize(2):

  Cuts lower bit and Median-Cut.

* Clean RGB:

  Sets R, G, B pixel color values into 0 if the alpha value is 0.

* Clean Palette:

  Removes unused palette.

* PNG zlib parameter

  It tries 4 strategies (``Z_DEFAULT_STRATEGY``, ``Z_FILTERED``, ``Z_RLE``, ``Z_HUFFMAN_ONLY``,
  not ``Z_FIXED``) x 6 filter options and decide the smallest option.

* Zopfli

  There are two options. ``--optimize 2`` runs zlib otpion trial (``Z_DEFAULT_STRATEGY`` and all filter
  options) and then try Zopfli compression with the smallest filter. ``--optimize 3`` is a Bruto-force
  way. It tries all zlib options and all Zopfli options.

How To Build
------------

It needs following library. You should download and decompress at the root folder:

* `boost_1_53_0 <http://www.boost.org/>`_

It uses `Zopfli <https://code.google.com/p/zopfli/>`_, `ligimagequant <http://pngquant.org/lib/>`_
as submodule. At first you should run following command ::

   $ git submodule int
   $ git submodule update

It uses following libraries, but these source code are bundled:

* `libpng <http://www.libpng.org/pub/png/>`_ (modified version to use Zopfli)
* `libz <http://www.zlib.net/>`_
* `jpeglib <http://www.ijg.org/>`_
* `pthreads-win32 <http://www.sourceware.org/pthreads-win32/>`_ (for windows cross compile)

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

* ``-o`` *level*, ``--optimize`` *level*:

  Set optimize level:

  * ``0`` - No optimize (fastest)
  * ``1`` - PNG zlib option optimize + index color optimize(default)
  * ``2`` - Use zopfli with one fiilter + 1
  * ``3`` - Use zopfli with all filters + 1

  level 2 and 3 becomes same results almost all cases. Preview modes always use ``0``.

* ``-b``, ``--benchmark``:

  Display time to process.

* ``-v``, ``--verbose``:

  Display compression result.

* ``-h``, ``--help``: Show this message.

Output Options
~~~~~~~~~~~~~~

* ``-16m`` *PATH*:

  16 bit PNG with 1 bit alpha (RGBA 5551). If source image doesn't have alpha, it generates RGB 565 PNG.

* ``-16a`` *PATH*:

  16 bit PNG with 4 bit alpha (RGBA 4444). If source image doesn't have alpha, it generates RGB 565 PNG.

* ``-16`` *PATH*:

  It is as same as ``-16a`` .

* ``-16i`` *PATH*:

  16 bit PNG with 4 bit alpha (RGBA 4444) and 256 colors palette. If source image doesn't have alpha, it generates RGB 565 PNG.

* ``-32`` *PATH*:

  24/32 bit PNG. It tries several compression option and reduce file size.

* ``-32i`` *PATH*:

  24/32 bit PNG with 256 colors palette. It tries several compression option and reduce file size.

* ``-p16m`` *PATH*:

  Preview mode of ``-16m``.

* ``-p16a`` *PATH*:

  Preview mode of ``-16a``.

* ``-p16`` *PATH*:

  Preview mode of ``-16``.

* ``-p16i`` *PATH*:

  Preview mode of ``-16i``.

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

Sample
------

All samples are uploaded on `Flicker <http://www.flickr.com/photos/shibukawa/sets/72157633109826482/>`_.

**Lenna: Original File (512x512):** 525,521 bytes (converted from `tiff file <http://www-2.cs.cmu.edu/~chuck/lennapg/lena_std.tif>`_ by using Preview.app)

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8595702270/" title="lenna by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8384/8595702270_d5873869dd_n.jpg" width="320" height="320" alt="lenna"></a>

**Smooth UI Kit (400x300):** 103,410 bytes (converted from `PSD file <http://www.icondeposit.com/design:52>`_ by using Photoshop CS5)

.. raw:: html

Lossless Result
~~~~~~~~~~~~~~~

**Full color PNG:**

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8594604001/" title="lenna_32 by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8525/8594604001_0429ce1d85.jpg" width="500" height="500" alt="lenna_32"></a>

* 473,226 bytes (``lightpng lenna.png -o 2 -32 lenna_32.png``)

.. raw:: html

* 75,570 bytes (``lightpng uikit.png -o 2 -32 uikit_32.png``)

Lossy Results
~~~~~~~~~~~~~

**16bit(RGB565) PNG:**

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8594603339/" title="lenna_16 by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8086/8594603339_dc291b3177.jpg" width="500" height="500" alt="lenna_16"></a>

* 274,089 bytes (``lightpng lenna.png -o 2 -16 lenna_16.png``)

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8597500384/" title="uikit_16 by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8108/8597500384_4b78b82b93.jpg" width="400" height="300" alt="uikit_16"></a>

* 59,532 bytes (``lightpng uikit.png -o 2 -16 uikit_16.png``)

**24bit index color PNG:**

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8594602991/" title="lenna_32i by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8097/8594602991_c4443d0608.jpg" width="500" height="500" alt="lenna_32i"></a>

* 178,214 bytes (``lightpng lenna.png -o 2 -32i lenna_32i.png``)

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8596394929/" title="uikit_32i by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8091/8596394929_1eea9d7374.jpg" width="400" height="300" alt="uikit_32i"></a>

* 56,506 bytes (``lightpng uikit.png -o 2 -32i uikit_32i.png``)

**16bit(RGB565) index color PNG:**

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8594603239/" title="lenna_16i by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8509/8594603239_3915f12bed.jpg" width="500" height="500" alt="lenna_16i"></a>

* 153,976 bytes (``lightpng lenna.png -o 2 -16i lenna_16i.png``)

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8597500392/" title="uikit_16i by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8105/8597500392_26db422844.jpg" width="400" height="300" alt="uikit_16i"></a>

* 34,289 bytes (``lightpng uikit.png -o 2 -16i uikit_16i.png``)

Non-Open Source Version
-----------------------

This program experimentally supports compressed texture generation (pvr/atc). It needs PVRTexLib and/or AdrenoSDK. They are available here:

* http://www.imgtec.com/powervr/insider/powervr-utilities.asp
* http://developer.qualcomm.com/develop/

AdrenoSDK is proveded in Windows execution. So you need wine to install and extract needed libraries/headers.

To create this version, set following flags to scons command:

* ``--no-opensource``:

  Enable Texture Compression format support

* ``--PVRTexLib=``\ *DIR*:

  Enable PVRTC Texture Compression convert/preview feature. Default is "./PVRTexLib".

* ``--AdrenoSDK=``\ *DIR*:

  Enable ATITC Texture Compression convert/preview feature. Default is "~/.wine/drive_c/AdrenoSDK".

It adds following output options:

* ``-pvr`` *PATH*:

  4 bpp PVRTC compressed texture file (PVR version 3). PVRTC should be square powered by two sizes. ``lightpng`` adds extra spaces to match this rule.

* ``-lpvr`` *PATH*:

  4 bpp PVRTC compressed texture file with legacy format (PVR version 2).

  It is a same format as Apple's ``texturetool`` command.

* ``-ppvr`` *PATH*:

  Preview mode of PVRTC.

* ``-atc`` *PATH*:

  8 bpp ATITC compressed texture file. It is a just raw bit data. It doesn't contain picture size information.

* ``-fatc`` *PATH*:

  8 bpp ATITC compressed texture file with header information.

* ``-patc`` *PATH*:

  Preview mode of PVRTC

If you use this option, use this program for internal use. You can't distribute your modified source code and/or binary in public.
If you want to reuse my source code to your product, please sent me Amazon.com or Amazon.co.jp e-mail gift to my address (yoshiki at shibu.jp).

I am reading the license of Ardeno SDK, but it maybe prohibit linking with open source tool (legal English is very difficult). If I find any good way to link with open source code, I will merge this code into main.

.. note::

   ``--no-opensource`` and ``--mingw32`` can not be enabled at the same time now.
   Please anyone tell me how to use closed source .lib via mingw32.

Texture Compression Sample (Preview Mode)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These resulting PNG files are converted from PVR/ATC. They are good for previewing qualities.

**PVR preview:**

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8596237243/" title="lenna_pvr by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8524/8596237243_253c3b80aa.jpg" width="500" height="500" alt="lenna_pvr"></a>

* (``lightpng lenna.png -ppvr lenna_pvr.png``)

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8597500328/" title="uikit_pvr by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8516/8597500328_9f02e74c77.jpg" width="400" height="300" alt="uikit_pvr"></a>

* (``lightpng uikit.png -ppvr uikit_pvr.png``)

**ATC preview:**

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8596237149/" title="lenna_atc by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8519/8596237149_5e2f022dc4.jpg" width="500" height="500" alt="lenna_atc"></a>

* (``lightpng lenna.png -patc lenna_atc.png``)

.. raw:: html

   <a href="http://www.flickr.com/photos/shibukawa/8597500352/" title="uikit_atc by shibukawa.yoshiki, on Flickr"><img src="http://farm9.staticflickr.com/8228/8597500352_8ae0031895.jpg" width="400" height="300" alt="uikit_atc"></a>

* (``lightpng uikit.png -patc uikit_atc.png``)

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

