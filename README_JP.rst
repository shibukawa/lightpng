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

ngCoreでは、RGBA5551とRGB565をサポートしているため、このファイルをそのまま読み込ませることができます。

PNGは開発者も、デザイナーも簡単に利用できるフォーマットです。結果のPNGファイルは元のPNGよりも小さくなります。モバイルゲームの開発者に最適です。

ビルドの仕方
------------

ビルドには下記のライブラリが必要となります:

* libpng
* libz

このリポジトリには、MacPorts環境のMac OS XでビルドするためのSConsの設定ファイルが含まれています ::

   $ sudo port install libpng scons 
   $ scons
   $ ./lightpng input.png output.png

使用方法
--------

コマンドラインから次のように実行します ::

   $ lightpng [opt] input_image output_image

オプション
~~~~~~~~~~

|-t, --texture| テクスチャモード (デフォルト)
|-p, --preview| プレビューモード
|-m, --mask| RGBA 5551 / RGB 565 (デフォルト)
|-a, --alpha| RGBA 4444 (アルファチャンネルを持たないデータの場合は無視されます)
|-h, --help| 使い方を表示します

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

ライセンス
----------

このソースコードはMITライセンスでリリースされています。

.. include:: LICENSE.rst

作者
----

:Copyright: Yoshiki Shibukawa (DeNA Co.,Ltd. / ngmoco:) LLC)
:Contact: yshibukawa at ngmoco.com

謝辞
----

このソースコードは、家族、同僚(DeNA, ngmoco)などの多くの協力でできています。

本プログラムで利用しているディザリングのアルゴリズムはSc4Freakさんのブログ(http://adtsai.blogspot.com/2011/03/high-quality-dithering-for-windows.html) とソースコードで知りました。この実装は、Sc4FreakさんのWindows Phone用のプログラムを機能拡張し、移植性を高めたものです。

また、Greg Roelofs (libpngの作者) とJean-loup Gailly、Mark Adler (zlibの作者)のお陰で、１日で実装することができました。

