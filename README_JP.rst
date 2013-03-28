lightpng: ゲーム用のPNG最適化ツール
===================================

このツールの目的
----------------

このツールは、PNGファイルをゲームのテクスチャとして最適な形式に最適化します。

OpenGLやDirectXなどの3Dゲーム用のAPIは、RGBA5551、RGB565、RGBA4444などの効率のよいテクスチャフォーマットをいくつかサポートしています。省メモリとパフォーマンスが向上することが期待されます。

また、現在のゲーム用のAPIは、PNGファイルをテクスチャとして使うためのAPIを用意しています。ですが、PNGでは上記のような1ピクセルあたり16ビットのデータを格納することはできません。

このプログラムは、PNGファイルの色数を削減して、16ビットの色空間で表現できるように変換します。 `Floyd–Steinberg dithering <http://en.wikipedia.org/wiki/Floyd%E2%80%93Steinberg_dithering>`_ アルゴリズムを利用して、なるべく高画質なPNGファイルを生成します。次のようなコードを使って、簡単に16ビットのデータに変換することができます::

    // アルファチャンネルがなかった場合
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

ngCoreでは、RGBA4444とRGB565をサポートしているため、このファイルをそのまま読み込ませることができます。

PNGは開発者も、デザイナーも簡単に利用できるフォーマットです。結果のPNGファイルは元のPNGよりも小さくなります。モバイルゲームの開発者に最適です。

Why the File become Smaller? / What does this tool do?

なぜファイルサイズが小さくなるのか？このツールは何をしているのか？
------------------------------------------------------------------

.. image:: http://farm9.staticflickr.com/8367/8596428211_c454143237_z.jpg
   :target: http://www.flickr.com/photos/shibukawa/8596428211/

* 16 Bit Conversion:

  下位ビットを削除します。

* Dithering:

  Floyd-Steinbergのディザリングです。

* Quantize(1):

  `ligimagequant <http://pngquant.org/lib/>`_ を使用して減色します。

* Quantize(2):

  下位ビットを削り、メディアンカット法で減色します。

* Clean RGB:

  アルファがゼロのピクセルがあれば、R, G, Bの値をゼロにします。

* Clean Palette:

  未使用のパレットを削除します。

* PNG zlib parameter

  4種類の圧縮アルゴリズム (``Z_DEFAULT_STRATEGY``, ``Z_FILTERED``, ``Z_RLE``, ``Z_HUFFMAN_ONLY``,
  ただし ``Z_FIXED`` は使わない) x 6種類のフィルタの組み合わせで圧縮を行なってみて、最小のファイルサイズになるオプションを探します。

* Zopfli

  2種類のオプションがあります。 ``--optimize 2`` の時は、zlibのオプションをトライアルで行なってみて(``Z_DEFAULT_STRATEGY`` とすべてのフィルタオプション)、その中で最小の結果になったフィルタオプションを使ってZopfliを使った圧縮を行います。 ``--optimize 3`` の時は、すべてのzlibとZopfliのオプションの組み合わせでトライアルを行います。

ビルドの仕方
------------

下記のライブラリが必要となります。ダウンロードしてリポジトリのルートフォルダに展開してください。

* `boost_1_53_0 <http://www.boost.org/>`_

このプロジェクトでは `Zopfli <https://code.google.com/p/zopfli/>`_ と `ligimagequant <http://pngquant.org/lib/>`_ をサブモジュールとして使います。最初に下記のコマンドを実行してください ::

   $ git submodule init
   $ git submodule update

ビルドには下記のライブラリが必要となりますが、これらのソースコードは同梱しています:

* `libpng <http://www.libpng.org/pub/png/>`_ (Zopfliを使用するように改造済み)
* `libz <http://www.zlib.net/>`_
* `jpeglib <http://www.ijg.org/>`_
* `pthreads-win32 <http://www.sourceware.org/pthreads-win32/>`_ (Windows版のクロスコンパイル用)

このリポジトリには、SConsの設定ファイルがついています。MacPorts環境のMac OS Xでしかテストしてませんが、
他のSCons/gcc環境でもビルドできると思います ::

   $ sudo port install scons
   $ scons
   $ ./lightpng input.png -16m output.png

また、MinGWが使える環境があれば、Windows用のバイナリをクロスコンパイルすることもできます::

   $ sudo port install i386-mingw32-gcc scons
   $ soncs --mingw32
   $ wine lightpng.exe input.png -16m output.png

使用方法
--------

コマンドラインから次のように実行します ::

   $ lightpng [オプション] 入力画像 [出力オプション]

``入力画像`` は ``.png`` と ``.jpg`` 画像を受け取ることができます。


オプション
~~~~~~~~~~

:-s, --skip: PNGファイルのサイズ最適化をスキップします
:-b, --benchmark: 
:-v, --verbose: 
:-h, --help: 

Options
~~~~~~~

* ``-o`` *レベル*, ``--optimize`` *レベル*:

  最適化レベルを設定します:

  * ``0`` - 最適化なし(最速)
  * ``1`` - PNG zlibオプションの最適化と、インデックスカラーの最適化(デフォルト)
  * ``2`` - 1の最適化と、Zopfliと1つのフィルタを使って圧縮
  * ``3`` - Zopfliのとすべてのフィルタを使って圧縮

  ほとんどの場合で、レベル2とレベル3は同じ結果になります。また、プレビューモードは常に ``0`` を使用します。

* ``-b``, ``--benchmark``:

  処理にかかった時間を表示します

* ``-v``, ``--verbose``:

  圧縮結果を表示します

* ``-h``, ``--help``:
  
  使い方を表示します

出力オプション
~~~~~~~~~~~~~~

* ``-16m`` *パス*:
  
  1ビットのアルファチャンネルを持つ16ビットのPNG(RGBA 5551)を生成します。もし入力ファイルがアルファチャンネルを持っていなかった場合は、RGB 565のPNGファイルを生成します。

* ``-16a`` *パス*:

  4ビットのアルファチャンネルを持つ16ビットのPNG(RGBA 4444)を生成します。もし入力ファイルがアルファチャンネルを持っていなかった場合は、RGB 565のPNGファイルを生成します。

* ``-16`` *パス*:

  ``-16a`` と同じです

* ``-16i`` *パス*:

  4ビットのアルファチャンネルを持つ16ビットで、かつ256色以内のインデックスカラーを持つのPNG(RGBA 4444)を生成します。もし入力ファイルがアルファチャンネルを持っていなかった場合は、RGB 565のPNGファイルを生成します。

* ``-32`` *パス*:

  24/32ビットのPNGファイルを生成します。ディザリングは行わず、圧縮オプションを調整してファイルサイズの縮小のみを行います。

* ``-32i`` *パス*:

  24/32ビットの256色以内のインデックスカラーを持つPNGファイルを生成します。ディザリングは行わず、圧縮オプションを調整してファイルサイズの縮小のみを行います。

* ``-p16m`` *パス*:

  ``-16m`` のプレビューモードです。

* ``-p16a`` *パス*:

  ``-16a`` のプレビューモードです。

* ``-p16`` *パス*:

  ``-16`` のプレビューモードです。

* ``-p16i`` *パス*:

  ``-16i`` のプレビューモードです。

テクスチャモード / プレビューモード
-----------------------------------

最終的な出力データを作りたい場合には、テクスチャモードを利用してください。
このモードでは、不要となるビットを切り落とした結果を生成します。そのため、プレビューには適さないことがあります。

たとえば、RGBA 5551フォーマットの場合には、1ビットしかアルファチャンネルが含まれません。下記のようなソリッドカラーのピクセルがあったとしても、変換結果は半透明になります。

**変換前**

.. list-table::

   - * R
     * ``1100 0000``
   - * G
     * ``1111 1111``
   - * B
     * ``0000 1111``
   - * A
     * ``1111 1111``

**変換後**

.. list-table::

   - * R
     * ``1100 0000``
   - * G
     * ``1111 1000``
   - * B
     * ``0000 1000``
   - * A
     * ``1000 0000``

プレビューモードはこの欠けた情報を補完します。好きなツールを使って、最終イメージに近い結果を確認することができます。

サンプル
--------

すべてのサンプルは `Flicker <http://www.flickr.com/photos/shibukawa/sets/72157633109826482/>`_ にアップロードされています。

* **Lenna: Original File (512x512):** 525,521 bytes (converted from `tiff file <http://www-2.cs.cmu.edu/~chuck/lennapg/lena_std.tif>`_ by using Preview.app)

* **Smooth UI Kit: Original File (400x300):** 103,410 bytes (converted from `PSD file <http://www.icondeposit.com/design:52>`_ by using Photoshop CS5)

  This file is created by Matt Gentile. Released under Creative Commons Attribution 3.0

フルカラーPNG (ロスレス)
~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: http://farm9.staticflickr.com/8525/8594604001_f1ae0c37b4_o.png
   :target: http://www.flickr.com/photos/shibukawa/8594604001/

* 473,226 bytes (``lightpng lenna.png -o 2 -32 lenna_32.png``)

.. image:: http://farm9.staticflickr.com/8382/8596394907_626eb2967b_o.png
   :target: http://www.flickr.com/photos/shibukawa/8596394907/

* 75,570 bytes (``lightpng uikit.png -o 2 -32 uikit_32.png``)

16bit(RGB565) PNG (ロスあり)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: http://farm9.staticflickr.com/8086/8594603339_eee3b3c44a_o.png
   :target: http://www.flickr.com/photos/shibukawa/8594603339/

* 274,089 bytes (``lightpng lenna.png -o 2 -16 lenna_16.png``)

.. image:: http://farm9.staticflickr.com/8108/8597500384_a818719645_o.png
   :target: http://www.flickr.com/photos/shibukawa/8597500384/

* 59,532 bytes (``lightpng uikit.png -o 2 -16 uikit_16.png``)

24bitインデックスカラーPNG (ロスあり)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: http://farm9.staticflickr.com/8097/8594602991_4667dc59b8_o.png
   :target: http://www.flickr.com/photos/shibukawa/8594602991/

* 178,214 bytes (``lightpng lenna.png -o 2 -32i lenna_32i.png``)

.. image:: http://farm9.staticflickr.com/8091/8596394929_f639151bfc_o.png
   :target: http://www.flickr.com/photos/shibukawa/8596394929/

* 56,506 bytes (``lightpng uikit.png -o 2 -32i uikit_32i.png``)

16bit(RGB565) インデックスカラーPNG (ロスあり)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: http://farm9.staticflickr.com/8509/8594603239_ea5eaa07f7_o.png
   :target: http://www.flickr.com/photos/shibukawa/8594603239/

* 153,976 bytes (``lightpng lenna.png -o 2 -16i lenna_16i.png``)

.. image:: http://farm9.staticflickr.com/8105/8597500392_9567a3b8b1_o.png
   :target: http://www.flickr.com/photos/shibukawa/8597500392/

* 34,289 bytes (``lightpng uikit.png -o 2 -16i uikit_16i.png``)

クローズドソースバージョン
--------------------------

このプログラムは実験的に、圧縮テクスチャ(pvr/atc)の生成もサポートしています。これには、PVRTexLibとAdrenoSDK(どちらか、もしくは両方)が必要になります。

* http://www.imgtec.com/powervr/insider/powervr-utilities.asp
* http://developer.qualcomm.com/develop/

AdrenoSDKはWindowsの実行形式として提供されています。そのため、SDKを展開して、必要なライブラリやヘッダを取り出すにはwineが必要となります。

クローズドソース版を作成するには、以下のオプションをsconsコマンドに追加してください。

* ``--no-opensource``:

  圧縮テクスチャのサポートを有効にします。

* ``--PVRTexLib=``\ *DIR*:

  PVRTC圧縮テクスチャへの変換・プレビューを有効にします。デフォルトは "./PVRTexLib" です。 

* ``--AdrenoSDK=``\ *DIR*:

  ATITC圧縮テクスチャへの変換・プレビューを有効にします。デフォルトは "~/.wine/drive_c/AdrenoSDK" です。

このオプションを使ってビルドすると、出力オプションに次の項目が追加されます。

* ``-pvr`` *パス*:

  4 bpp PVRTC圧縮テクスチャ(PVRバージョン3)。PVRTCは2のべき乗の正方形である必要があります。 ``lightpng`` はこのルールにあうようにスペースを追加します。

* ``-lpvr`` *パス*:

  旧式の、4 bpp PVRTC圧縮テクスチャ(PVR version 2)です。

  このコマンドの結果は、アップルの ``texturetool`` と同じ結果になります。

* ``-ppvr`` *パス*:

  4 bpp PVRTC圧縮テクスチャのプレビューモードです。

* ``-atc`` *パス*:

  ATITC圧縮テクスチャです。このファイルの出力結果はビットデータのみです。絵のサイズの情報などは含まれません。

* ``-fatc`` *パス*:

  8 bpp ATITC圧縮テクスチャ(ヘッダ情報つき)です。AdrenoSDKのヘッダファイルを付与することで、イメージサイズの情報を後から取得することができます。

* ``-patc`` *パス*:

  8 bpp ATITC圧縮テクスチャのプレビューモード

このオプションを有効にして作成したプログラムは、一般公開せず、内部利用のみとしてください。改変したソースコードやバイナリも、一般公開は避けてください。もしこのクローズド版に含まれるコードを流用したい場合は、 yoshiki at shibu.jp まで、Amazon.comかAmazon.co.jpのギフトを送ってください。

現在、Ardeno SDKのライセンスを読んでいますが、オープンソースのプログラムとのリンクを制限するような事項があります。もし良い解決方法があれば、MITライセンスのコードの方にこの機能を取り入れる予定です。

.. note::

   現在は ``--no-opensource`` と ``--mingw32`` を同時に利用することはできません。
   もし、mingw32でクローズドソースの.libを利用する方法が分かる方はお知らせ下さい。

PVR圧縮テクスチャサンプル(プレビューモード)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: http://farm9.staticflickr.com/8524/8596237243_38d985ac86_o.png
   :target: http://www.flickr.com/photos/shibukawa/8596237243/

* (``lightpng lenna.png -ppvr lenna_pvr.png``)

.. image:: http://farm9.staticflickr.com/8516/8597500328_7fe762e7e1_o.png
   :target: http://www.flickr.com/photos/shibukawa/8597500328/

* (``lightpng uikit.png -ppvr uikit_pvr.png``)

ATC圧縮テクスチャサンプル(プレビューモード)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: http://farm9.staticflickr.com/8519/8596237149_9ca1a16736_o.png
   :target: http://www.flickr.com/photos/shibukawa/8596237149/

* (``lightpng lenna.png -patc lenna_atc.png``)

.. image:: http://farm9.staticflickr.com/8228/8597500352_60132247c9_o.png
   :target: http://www.flickr.com/photos/shibukawa/8597500352/

* (``lightpng uikit.png -patc uikit_atc.png``)

ライセンス
----------

このソースコードはMITライセンスでリリースされています。

.. include:: LICENSE.rst

作者
------

:Copyright: Yoshiki Shibukawa (DeNA Co.,Ltd. / ngmoco:) LLC)
:Contact: yshibukawa at ngmoco.com

謝辞
----

このソースコードは、家族、同僚(DeNA, ngmoco)などの多くの協力でできています。

本プログラムで利用しているディザリングのアルゴリズムはSc4Freakさんのブログ(http://adtsai.blogspot.com/2011/03/high-quality-dithering-for-windows.html) とソースコードで知りました。この実装は、Sc4FreakさんのWindows Phone用のプログラムを機能拡張し、移植性を高めたものです。

また、Greg Roelofs (libpngの作者) とJean-loup Gailly、Mark Adler (zlibの作者)のお陰で、１日で実装することができました。

