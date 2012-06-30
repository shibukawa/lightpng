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
AddOption('--PVRTexLib',
          dest='pvrtexlib',
          action='store',
          metavar='DIR',
          default='./PVRTexLib',
          help='Enable PVRTC Texture Compression convert/preview feature. Default is "./PVRTexLib".')
AddOption('--AdrenoSDK',
          dest='adrenosdk',
          action='store',
          metavar='DIR',
          default='$HOME/.wine/drive_c/AdrenoSDK',
          help='Enable ATITC Texture Compression convert/preview feature. Default is "~/.wine/drive_c/AdrenoSDK".')

sources = ['src/PNGReader.cpp', 'src/JPEGReader.cpp', 'src/PNGWriter.cpp']
libs = ['png', 'z', 'jpeg']
libpath = ['third_party/zlib/', 'third_party/libpng/', 'third_party/jpeg/']
cpppath = ['src', 'third_party/zlib/', 'third_party/libpng/', 'third_party/jpeg/']
ccflags = []

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
    ccflags.append('-O4')
else:
    ccflags.append('-O4')


if GetOption('mingw32'):
    env.Tool('crossmingw', toolpath=['tool'])
    print("@@@ enable mingw32")
    build_target = "win"
    ccflags.append('-D__QT__')

if not GetOption('opensource'):
    import os
    enable_texture = False

    sources.insert(0, 'no_opensource/lightpng.cpp')
    cpppath.append("./no_opensource/")

    pvrtexlib = fix_path(GetOption('pvrtexlib'))
    if os.path.exists(pvrtexlib + '/PVRTexture.h'):
        print("@@@ enable PVRTC convert/preview feature")
        sources.append('no_opensource/PVRWrite.cpp')
        cpppath.append(pvrtexlib)
        libs.append('PVRTexLib')
        ccflags.append('-DPVRTC')
        if build_target == "win":
            libpath.append(pvrtexlib + '/Windows_x86_32/Lib/')
        else:
            libpath.append(pvrtexlib + '/MacOS_x86/')
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
        sources.append('no_opensource/ATCWrite.cpp')
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

env.SConscript(dirs=['third_party/zlib', 'third_party/libpng', 'third_party/jpeg'])
env.Program('lightpng', sources, LIBS=libs, LIBPATH=libpath, CPPPATH=cpppath, CCFLAGS=ccflags)
