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
#include <cstddef>
#include <string>


typedef enum ZlibCompressionLevel {
    ZLIB_NONE	= ~(0x00),
    ZLIB_NORM	= (0x00),
    ZLIB_FAST	= (0x01),
    ZLIB_BEST	= (0x09),
} ZlibCompression;

class ZHandle {
    public:
        int zlib_inflate(std::string inFile, std::string outFile);
        int zlib_deflate(std::string inFile, std::string outFile, ZlibCompression level);
        int zlib_scan(std::string inFile, int level);

    private:
        bool zlib_try(std::string inFile, long int size, long int offset);
};
