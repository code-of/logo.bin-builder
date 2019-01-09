project('logo-builder','cpp')

add_project_arguments(
                      [
                        '-std=gnu++2a',
                        '-Wall', 
                        '-pedantic', 
                        '-Wno-deprecated',
                        '-m64',
                        '-mtune=generic',
                        '-O2',
                        '-Os',
                        '-pthread',
                        '-fopenmp',
                        '-fstack-protector-strong',
                        '-fno-plt'
                      ], language : 'cpp')

hh = include_directories('./include')
header = declare_dependency(include_directories : hh)
zlib = dependency('zlib')
lcms = dependency('lcms')
threads = dependency('threads')
GraphicsMagick = dependency('GraphicsMagick')
GraphicsMagickCpp = dependency('GraphicsMagick++')
Gtkmm = dependency('gtkmm-2.4')

executable('logo-builder', 
            [
              'src/cfile.cc', 
              'src/zlib.cc', 
              'src/magick.cc',
              'src/builder.cc',
              'src/cli.cc',
            ],
            link_args : 
            '-Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now',
            dependencies : 
            [
              header,
              GraphicsMagick,
              GraphicsMagickCpp,
              threads,
              lcms,
              zlib
              ]
          )