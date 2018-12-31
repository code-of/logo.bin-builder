#/* !zlib.cc */
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
#include <zlib.hh>
#include <unistd.h>
#include <fcntl.h>
#include <zlib.h>

using namespace std;

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

int ZHandle::zlib_inflate(string inFile, string outFile)
{
    int ret = 0;
    z_stream stream;
    Byte ibuf[CHUNK];
    Byte obuf[CHUNK];
    unsigned int have_bytes = 0;

    if (zini(&stream, 0, ZLIB_NONE)) {
        FILE *input = fopen(inFile.c_str(), "rb+");
        FILE *output = fopen(outFile.c_str(), "wb+");

        if (input && output) {
            do {
                stream.avail_in = fread(ibuf, 1, CHUNK, input);

                if (ferror(input))
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

                    if ((fwrite(obuf, 1, have_bytes, output) != have_bytes) || ferror(output))
                        goto _inf_zfini;
                } while (0 == stream.avail_out);
            } while (Z_STREAM_END != ret);
_inf_zfini:
            if ((NULL != stream.msg) && zlog)
                perror(stream.msg);
            fflush(output);
            fflush(input);
            fclose(output);
            fclose(input);
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

int ZHandle::zlib_deflate(string inFile, string outFile, ZlibCompression level)
{
    int ret = 0;
    int flush = 0;
    z_stream stream;
    Byte ibuf[CHUNK];
    Byte obuf[CHUNK];
    unsigned int have_bytes = 0;

    if (zini(&stream, 1, level)) {
        FILE *input = fopen(inFile.c_str(), "rb+");
        FILE *output = fopen(outFile.c_str(), "wb+");
        if (input && output) {
            do {
                stream.avail_in = fread(ibuf, 1, CHUNK, input);

                if (ferror(input))
                    goto _def_zfini;

                flush = feof(input)
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
                    if ((fwrite(obuf, 1, have_bytes, output) != have_bytes) || ferror(output))
                        goto _def_zfini;
                } while (0 == stream.avail_out);
                if (0 != stream.avail_in)
                    exit(EXIT_FAILURE);
            } while (Z_FINISH != flush);
_def_zfini:
            if (NULL != stream.msg)
                perror(stream.msg);
            fflush(output);
            fflush(input);
            fclose(output);
            fclose(input);
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

int ZHandle::zlib_scan(string inFile, int level)
{
    long le = 0;
    int matches = 0;
    Byte byte = (Byte)0x00;
    Byte zmagic[2] = { CMF, FLG(level) };
    FILE *file = fopen(inFile.c_str(), "rb");

    if (NULL != file) {
        fseek(file, 0, SEEK_END);
        le = ftell(file);
        fseek(file, 0, SEEK_SET);

        for (long i = 0; i < le; i++) {
            if (zmagic[0] == (byte = fgetc(file))) {
                if (zmagic[1] == (byte = fgetc(file))) {
                    if (this->zlib_try(inFile, le, i))
                        matches++;
                    fseek(file, i, SEEK_SET);
                } else {
                    fseek(file, i, SEEK_SET);
                }
            }
        }
    }

    fflush(file);
    fclose(file);
    return matches;
}

bool ZHandle::zlib_try(string inFile, long int size, long int offset)
{
    Byte buf = 0x00;
    long int iter = (size - offset);
    string rgba = to_string(offset - 1).append(".rgba");
    string zblob = to_string(offset - 1).append(".zlib");
    FILE *stream = fopen(zblob.c_str(), "wb+");
    FILE *binfile = fopen(inFile.c_str(), "rb");

    fseek(binfile, offset - 1, SEEK_SET);

    for (; iter != 0; iter--) {
        buf = fgetc(binfile);
        fputc(buf, stream);
    }

    fflush(binfile);
    fflush(stream);
    fclose(binfile);
    fclose(stream);

    zlog = false;
    this->zlib_inflate(zblob, rgba);
    zlog = true;

    unlink(zblob.c_str());

    if (0 == access(rgba.c_str(), F_OK)) {
        return true;
    } else {
        unlink(rgba.c_str());
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
