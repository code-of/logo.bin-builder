#pragma once
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
static_assert(__GNUG__, "Your compiler is not supporting GnuExtensions !");
#define __Begin           extern "C" {
#define __End             }

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <string>


__Begin
#include <fcntl.h>
#include <zlib.h>
#include <sys/types.h>
__End

namespace zlib {
    typedef z_stream zstream;

    enum CompressionLevel {
        ZLIB_NONE	= ~(0x00),
        ZLIB_NORM	= (0x00),
        ZLIB_FAST	= (0x01),
        ZLIB_BEST	= (0x09),
    };

    constexpr u_int32_t CHUNK = (0x4000U);

    constexpr Byte CMF = ((Byte)0x78);

    constexpr Byte FLG(int level)
    {
        return (level > 1)
               ? ((level > 5)
                  ? ((level < 7)
                     ?  ((Byte)0x9C)
                     : ((Byte)0xDA))
                  : ((Byte)0x5E))
               : ((Byte)0x01);
    }

    class ZConverter {
        public:
            int zinflate(std::string inFile, std::string outFile);
            int zdeflate(std::string inFile, std::string outFile);
            int zscan(std::string binFile);

        protected:
            bool ztry(std::string binary, long int size, long int offset);
            void zini(zstream *stream, unsigned int mode);
            void zfini(zstream *stream, unsigned int mode);
    };
}
