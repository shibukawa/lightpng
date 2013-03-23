import sys
env = Environment();
Export('env')
AddOption('--enable-debug',
          dest='enabledebug',
          action='store_true',
          default=False,
          help='Debug Build')
AddOption('--mingw32',
          dest='mingw32',
          action='store_true',
          default=False,
          help='Create Windows binary by using MinGW')
AddOption('--no-opensource',
          dest='opensource',
          action='store_false',
          default=True,
          help='Enable Texture Compression format support')

if sys.platform == 'win32':
    default_pvr_path = r'C:\Imagination\PowerVR\GraphicsSDK\PVRTexTool\Library'
elif sys.platform == 'darwin':
    default_pvr_path = r'/Applications/Imagination/PowerVR/GraphicsSDK/PVRTexTool/Library'
AddOption('--PVRTexLib',
          dest='pvrtexlib',
          action='store',
          metavar='DIR',
          default=default_pvr_path,
          help='Enable PVRTC Texture Compression convert/preview feature. Default is "./PVRTexLib".')
AddOption('--AdrenoSDK',
          dest='adrenosdk',
          action='store',
          metavar='DIR',
          default='$HOME/.wine/drive_c/AdrenoSDK',
          help='Enable ATITC Texture Compression convert/preview feature. Default is "~/.wine/drive_c/AdrenoSDK".')

sconscript = ['third_party/zlib', 'third_party/libpng', 'third_party/jpeg', 'third_party/pngnq', 'third_party/zopfli_build']
sources = ['src/PNGReader.cpp', 'src/JPEGReader.cpp', 'src/PNGWriter.cpp', 'src/MedianCutQuantizer.cpp', 'src/NeuralNetQuantizer.cpp', 'src/PaletteOptimizer.cpp']
libs = ['png', 'z', 'jpeg', 'pngnq', 'zopfli']
libpath = ['third_party/zlib/', 'third_party/libpng/', 'third_party/jpeg/', 'third_party/pngnq', 'third_party/zopfli_build/']
cpppath = [
    'boost_1_53_0/',
    'src/', 'third_party/zlib/', 'third_party/libpng/', 'third_party/jpeg/', 'third_party/pngnq/src/', 'third_party/zopfli/']
ccflags = []
linkflags = []

def fix_path(path):
    import os
    for key, value in os.environ.items():
        envkey = '$' + key
        if envkey in path:
            path = path.replace(envkey, value)
    return path

build_target = "mac"

if GetOption('enabledebug'):
    ccflags.append('-g')
    ccflags.append('-Wall')
else:
    ccflags.append('-O3')


if GetOption('mingw32'):
    env.Tool('crossmingw', toolpath=['tool'])
    print("@@@ enable mingw32")
    build_target = "win"
    ccflags.append('-D__QT__')
    sconscript.append('third_party/pthread-w32')
    cpppath.append('third_party/pthread-w32/')
    libpath.append('third_party/pthread-w32/')
    ccflags.append('-DPTW32_STATIC_LIB')
    libs.append('pthread')
else:
    ccflags.append('-m32')
    linkflags.append('-mmacosx-version-min=10.6')
    linkflags.append('-m32')

if not env.GetOption('opensource'):
    import os
    enable_texture = False

    sources.insert(0, 'no_opensource/lightpng.cpp')
    cpppath.append("./no_opensource/")

    pvrtexlib = fix_path(GetOption('pvrtexlib'))
    if os.path.exists(pvrtexlib + '/Include/PVRTexture.h'):
        print("@@@ enable PVRTC convert/preview feature")
        sources.append('no_opensource/PVRWriter.cpp')
        cpppath.append(pvrtexlib + '/Include')
        libs.append('PVRTexLib')
        ccflags.append('-DPVRTC')
        if build_target == "win":
            libpath.append(pvrtexlib + r'\\Windows_x86_32\\Lib')
        else:
            libpath.append(pvrtexlib + '/OSX_x86/Static/')
        enable_texture = True
    else:
        print("@@@ PVRTC is not supported @@@")
        print("   To enable PVRTC convert/preview feature, download PVRTexLib from Imagination")
        print("   and set path with --PVRTexLib option.\n")
        print("     http://www.imgtec.com/powervr/insider/powervr-utilities.asp")

    adrenosdk = fix_path(GetOption('adrenosdk'))
    convpath = os.path.relpath(adrenosdk + '/Tools/Texture Converter/')
    if os.path.exists(convpath):
        print("@@@ enable ATITC convert/preview feature")
        sources.append('no_opensource/ATCWriter.cpp')
        cpppath.append(convpath + "/inc/")
        ccflags.append('-DATITC')
        if build_target == "win":
            libs.append('TextureConverter')
            libpath.append(convpath + '/lib/win32/')
        else:
            libs.append('TextureConverterOSX')
            libpath.append(convpath + '/lib/osx/Release/')
        enable_texture = True
    else:
        print("@@@ ATITC is not supported @@@")
        print("   To enable ATITC convert/preview feature, download AdrenoSDK from Qualcomm")
        print("   and set path with --AdrenoSDK option.")
        print("   To extract library, you need Windows or wine.\n")
        print("     http://developer.qualcomm.com/develop/")

    if enable_texture:
        ccflags.append('-DTEXTURE')
else:
    sources.insert(0, 'src/lightpng.cpp')

env.SConscript(dirs=sconscript)
env.Program('lightpng', sources, LIBS=libs, LIBPATH=libpath, CPPPATH=cpppath, CCFLAGS=ccflags, LINKFLAGS=linkflags)
