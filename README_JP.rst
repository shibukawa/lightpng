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

ビルドの仕方
------------

ビルドには下記のライブラリが必要となりますが、どちらのソースコードも同梱しています:

* libpng
* libz
* jpeglib

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
:-b, --benchmark: 処理にかかった時間を表示します
:-v, --verbose: 圧縮結果を表示します
:-h, --help: 使い方を表示します

出力オプション
~~~~~~~~~~~~~~

:-16m PATH: 1ビットのアルファチャンネルを持つ16ビットのPNG(RGBA 5551)を生成します。もし入力ファイルがアルファチャンネルを持っていなかった場合は、RGB 565のPNGファイルを生成します。
:-16a PATH: 4ビットのアルファチャンネルを持つ16ビットのPNG(RGBA 4444)を生成します。もし入力ファイルがアルファチャンネルを持っていなかった場合は、RGB 565のPNGファイルを生成します。
:-16 PATH: ``-16a`` と同じです
:-32 PATH: 24/32ビットのPNGファイルを生成します。ディザリングは行わず、圧縮オプションを調整してファイルサイズの縮小のみを行います。
:-p16m PATH: ``-16m`` のプレビューモードです。
:-p16a PATH: ``-16a`` のプレビューモードです。
:-p16 PATH: ``-16`` のプレビューモードです。

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

クローズドソースバージョン
--------------------------

このプログラムは実験的に、圧縮テクスチャ(pvr/atc)の生成もサポートしています。これには、PVRTexLibとAdrenoSDK(どちらか、もしくは両方)が必要になります。

* http://www.imgtec.com/powervr/insider/powervr-utilities.asp
* http://developer.qualcomm.com/develop/

AdrenoSDKはWindowsの実行形式として提供されています。そのため、SDKを展開して、必要なライブラリやヘッダを取り出すにはwineが必要となります。

クローズドソース版を作成するには、以下のオプションをsconsコマンドに追加してください。

:--no-opensource: 圧縮テクスチャのサポートを有効にします。
:--PVRTexLib=DIR: PVRTC圧縮テクスチャへの変換・プレビューを有効にします。デフォルトは "./PVRTexLib" です。 
:--AdrenoSDK=DIR: ATITC圧縮テクスチャへの変換・プレビューを有効にします。デフォルトは "~/.wine/drive_c/AdrenoSDK" です。

このオプションを使ってビルドすると、出力オプションに次の項目が追加されます。

:-pvr PATH: 4 bpp PVRTC圧縮テクスチャ 
:-lpvr PATH: 4 bpp PVRTC圧縮テクスチャ(version 2のレガシーフォーマット)
:-ppvr PATH: 4 bpp PVRTC圧縮テクスチャのプレビューモード
:-atc PATH: 8 bpp ATITC圧縮テクスチャ
:-fatc PATH: 8 bpp ATITC圧縮テクスチャ(ヘッダ情報つき)
:-patc PATH: 8 bpp ATITC圧縮テクスチャのプレビューモード

このオプションを有効にして作成したプログラムは、一般公開せず、内部利用のみとしてください。改変したソースコードやバイナリも、一般公開は避けてください。もしこのクローズド版に含まれるコードを流用したい場合は、 yoshiki at shibu.jp まで、Amazon.comかAmazon.co.jpのギフトを送ってください。

現在、Ardeno SDKのライセンスを読んでいますが、オープンソースのプログラムとのリンクを制限するような事項があります。もし良い解決方法があれば、MITライセンスのコードの方にこの機能を取り入れる予定です。

.. note::

   現在は ``--no-opensource`` と ``--mingw32`` を同時に利用することはできません。
   もし、mingw32でクローズドソースの.libを利用する方法が分かる方はお知らせ下さい。

作者
----

:Copyright: Yoshiki Shibukawa (DeNA Co.,Ltd. / ngmoco:) LLC)
:Contact: yshibukawa at ngmoco.com

謝辞
----

このソースコードは、家族、同僚(DeNA, ngmoco)などの多くの協力でできています。

本プログラムで利用しているディザリングのアルゴリズムはSc4Freakさんのブログ(http://adtsai.blogspot.com/2011/03/high-quality-dithering-for-windows.html) とソースコードで知りました。この実装は、Sc4FreakさんのWindows Phone用のプログラムを機能拡張し、移植性を高めたものです。

また、Greg Roelofs (libpngの作者) とJean-loup Gailly、Mark Adler (zlibの作者)のお陰で、１日で実装することができました。

