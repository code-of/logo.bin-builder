#pragma once
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
static_assert(__GNUG__, "Your compiler is not supporting GnuExtensions !");
#/**/ define/**/ __BEGIN_DECLS      extern "C" {
#/**/ define/**/ __END_DECLS	    }
#/**/ undef  /**/ __cplusplus
#/**/ define /**/ __cplusplus       201812L
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <cstddef>
#include <string>
#include <zlib.h>

enum ZlibCompressionLevel {
    ZLIB_NONE	= ~(0x00),
    ZLIB_NORM	= (0x00),
    ZLIB_FAST	= (0x01),
    ZLIB_BEST	= (0x09),
};

class ZHandle {
    public:
        int zinflate(std::string inFile, std::string outFile);
        int zdeflate(std::string inFile, std::string outFile);
        int zscan(std::string binFile);

    protected:
        bool ztry(std::string binary, long int size, long int offset);
        void zini(z_stream *stream, unsigned int mode);
        void zfini(z_stream *stream, unsigned int mode);
};
