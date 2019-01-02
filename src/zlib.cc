#/* !zlib.cc */
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <zlib.h>
#include <zlib.hh>
#include <cfile.hh>

using namespace std;
using namespace file;

namespace zlib {
    static constexpr unsigned int CHUNK = (0x4000U);
    static constexpr Byte CMF = ((Byte)0x78);
    static constexpr Byte FLG(int level)
    {
        return (level > 1)
               ? ((level > 5)
                  ? ((level < 7)
                     ?  ((Byte)0x9C)
                     : ((Byte)0xDA))
                  : ((Byte)0x5E))
               : ((Byte)0x01);
    }

    static bool zini(z_stream *stream, unsigned int mode, ZlibCompression level);
    static void zfini(z_stream *stream, unsigned int mode);
    static bool zlog = true;

    int Handle::zlib_inflate(string inFile, string outFile)
    {
        int ret = 0;
        z_stream stream;
        Byte ibuf[CHUNK];
        Byte obuf[CHUNK];
        unsigned int have_bytes = 0;

        if (zini(&stream, 0, ZLIB_NONE)) {
            File input(inFile.c_str(), "rb+");
            File output(outFile.c_str(), "wb+");

            if (input.is_open() && output.is_open()) {
                do {
                    stream.avail_in = input.read(ibuf, CHUNK);

                    if (input.has_error())
                        goto _inf_zfini;

                    if (0 == stream.avail_in)
                        break;

                    stream.next_in = ibuf;
                    do {
                        stream.avail_out = CHUNK;
                        stream.next_out = (Byte *)obuf;
                        ret = inflate(&stream, Z_NO_FLUSH);

                        if (Z_STREAM_ERROR == ret)
                            goto _inf_zfini;

                        switch (ret) {
                        case Z_NEED_DICT:
                            ret = Z_DATA_ERROR;
                        case Z_DATA_ERROR:
                        case Z_MEM_ERROR:
                            goto _inf_zfini;
                        }

                        have_bytes = CHUNK - stream.avail_out;

                        if ((output.write(obuf, have_bytes) != have_bytes)
                            || output.has_error())
                            goto _inf_zfini;
                    } while (0 == stream.avail_out);
                } while (Z_STREAM_END != ret);
_inf_zfini:
                if ((NULL != stream.msg) && zlog)
                    error(stream.msg);
                output.close();
                input.close();
                zfini(&stream, 0);
                return Z_STREAM_END == ret
                       ? EXIT_SUCCESS
                       : EXIT_FAILURE;
            } else {
                zfini(&stream, 0);
            }
        }
        return EXIT_FAILURE;
    }

    int Handle::zlib_deflate(string inFile, string outFile, ZlibCompression level)
    {
        int ret = 0;
        int flush = 0;
        z_stream stream;
        Byte ibuf[CHUNK];
        Byte obuf[CHUNK];
        unsigned int have_bytes = 0;

        if (zini(&stream, 1, level)) {
            File input(inFile.c_str(), "rb+");
            File output(outFile.c_str(), "wb+");
            if (input.is_open() && output.is_open()) {
                do {
                    stream.avail_in = input.read(ibuf, CHUNK);

                    if (input.has_error())
                        goto _def_zfini;

                    flush = input.eof()
                            ? Z_FINISH
                            : Z_NO_FLUSH;

                    stream.next_in = ibuf;
                    do {
                        stream.avail_out = CHUNK;
                        stream.next_out = (Byte *)obuf;
                        ret = deflate(&stream, flush);
                        if (Z_STREAM_ERROR == ret)
                            goto _def_zfini;


                        have_bytes = CHUNK - stream.avail_out;
                        if ((output.write(obuf, have_bytes) != have_bytes)
                            || output.has_error())
                            goto _def_zfini;
                    } while (0 == stream.avail_out);
                    if (0 != stream.avail_in)
                        exit(EXIT_FAILURE);
                } while (Z_FINISH != flush);
_def_zfini:
                if (NULL != stream.msg)
                    error(stream.msg);
                output.close();
                input.close();
                zfini(&stream, 1);
                return Z_STREAM_END == ret
                       ? EXIT_SUCCESS
                       : EXIT_FAILURE;
            } else {
                zfini(&stream, 1);
            }
        }
        return EXIT_FAILURE;
    }

    int Handle::zlib_scan(string inFile, int level)
    {
        long le = 0;
        int matches = 0;
        Byte byte = (Byte)0x00;
        Byte zmagic[2] = { CMF, FLG(level) };
        File file(inFile.c_str(), "rb");

        if (file.is_open()) {
            le = file.size();

            for (long i = 0; i < le; i++) {
                if (zmagic[0] == (byte = file.getc())) {
                    if (zmagic[1] == (byte = file.getc())) {
                        if (this->zlib_try(inFile, le, i))
                            matches++;
                        file.seek(i);
                    } else {
                        file.seek(i);
                    }
                }
            }
        }

        file.close();
        return matches;
    }

    bool Handle::zlib_try(string inFile, long size, long offset)
    {
        Byte buf = 0x00;
        long iter = (size - offset);
        string rgba = to_string(offset - 1).append(".rgba");
        string zblob = to_string(offset - 1).append(".zlib");
        File stream(zblob.c_str(), "wb+");
        File binfile(inFile.c_str(), "rb");

        binfile.seek(offset - 1);

        for (; iter != 0; iter--) {
            buf = binfile.getc();
            stream.putc(buf);
        }

        binfile.close();
        stream.close();

        zlog = false;
        this->zlib_inflate(zblob, rgba);
        zlog = true;

        trash(zblob.c_str());

        if (exists(rgba.c_str())) {
            return true;
        } else {
            trash(rgba.c_str());
            return false;
        }
    }

    static bool zini(z_stream *stream, unsigned int mode, ZlibCompression level)
    {
        stream->zalloc = Z_NULL;
        stream->zfree = Z_NULL;
        stream->opaque = Z_NULL;

        switch (mode) {
        case 0:
            stream->avail_in = Z_NULL;
            stream->next_in = Z_NULL;
            return !inflateInit(stream);
            break;

        case 1:
            return !deflateInit(stream, level);
            break;

        default:
            return false;
        }
    }

    static void zfini(z_stream *stream, unsigned int mode)
    {
        switch (mode) {
        case 0:
            (void)inflateEnd(stream);
            break;

        case 1:
            (void)deflateEnd(stream);
            break;
        }
    }
}
