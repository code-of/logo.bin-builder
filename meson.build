project(
  'logo-builder', 
  'cpp',
  version : '1.3.0',
  license : 'MIT',
  default_options : [
    'warning_level=3',
    'cpp_std=gnu++2a',
    'prefix=/usr'
    ]
)

zlib = dependency(
  'zlib', 
  static: true, 
  required: true
)
lcms = dependency(
  'lcms', 
  required: true
)
threads = dependency(
  'threads', 
  required: true
)
GMagick = dependency(
  'GraphicsMagick++',
  required: true)
incldir = include_directories(
  './include'
)
'''
Gtkmm = dependency('gtkmm-2.4')
'''

deps_cli = declare_dependency(
  include_directories : [ incldir ],
  dependencies : [ GMagick, threads, lcms, zlib ],
)

builder_cli = executable(
 'logo-builder', [ 'src/cfile.cc', 'src/zlib.cc', 'src/magick.cc', 'src/builder.cc', 'src/cli.cc' ], 
 cpp_args : ['-std=gnu++2a', '-Wall', '-pedantic', '-Wno-deprecated', 
 '-m64', '-mtune=generic', '-O2', '-Os', '-pthread', 
 '-fopenmp', '-fstack-protector-strong', '-fno-plt' ], 
 link_args : [ '-Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now', '-lm' ], 
 dependencies : deps_cli , 
 install : true
)

test('Print Usage',builder_cli)