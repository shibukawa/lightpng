env = Environment();
Export('env')
env.SConscript(dirs=['third_party/zlib', 'third_party/libpng', 'third_party/jpeg'])
env.Program('lightpng',
        ['src/lightpng.cpp', 'src/PNGRead.cpp', 'src/JPEGRead.cpp'],
        LIBS=['png', 'z', 'jpeg'],
        LIBPATH=['third_party/zlib/', 'third_party/libpng/', 'third_party/jpeg/'],
        CPPPATH=['./src', 'third_party/zlib/', 'third_party/libpng/', 'third_party/jpeg/']);
