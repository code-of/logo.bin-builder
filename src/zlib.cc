#/* !zlib.cc */
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
#include <zlib.hh>
#include <fcntl.h>
#include <cassert>

using namespace std;

constexpr unsigned int CHUNK = (0x4000U);

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

int ZHandle::zinflate(string inFile, string outFile)
{
    int ret = 0;
    z_stream stream;
    Byte ibuf[CHUNK];
    Byte obuf[CHUNK];
    unsigned int have_bytes = 0;

    this->zini(&stream, 0x1F);
    FILE *input = fopen(inFile.c_str(), "rb+");
    FILE *output = fopen(outFile.c_str(), "wb+");
    if (input && output) {
        do {
            stream.avail_in = fread(ibuf, 1, CHUNK, input);

            if (ferror(input)) {
                (void)inflateEnd(&stream);
                return Z_ERRNO;
            }

            if (stream.avail_in == 0)
                break;

            stream.next_in = ibuf;
            do {
                stream.avail_out = CHUNK;
                stream.next_out = (Byte *)obuf;
                ret = inflate(&stream, Z_NO_FLUSH);
                if (Z_STREAM_ERROR == ret)
                    return ret;

                switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&stream);
                    return ret;
                }

                have_bytes = CHUNK - stream.avail_out;
                if ((fwrite(obuf, 1, have_bytes, output) != have_bytes) || ferror(output)) {
                    (void)inflateEnd(&stream);
                    return Z_ERRNO;
                }
            } while (0 == stream.avail_out);
        } while (Z_STREAM_END != ret);
        this->zfini(&stream, 0x1F);
        fclose(output);
        fclose(input);
        return Z_STREAM_END == ret
               ? Z_OK
               : Z_DATA_ERROR;
    } else {
        fclose(output);
        fclose(input);
    }
    this->zfini(&stream, 0x1F);
    return EXIT_FAILURE;
}

int ZHandle::zdeflate(string inFile, string outFile)
{
    int ret = 0;
    int flush = 0;
    z_stream stream;
    Byte ibuf[CHUNK];
    Byte obuf[CHUNK];
    unsigned int have_bytes = 0;

    this->zini(&stream, 0xDE);
    FILE *input = fopen(inFile.c_str(), "rb+");
    FILE *output = fopen(outFile.c_str(), "wb+");
    if (input && output) {
        do {
            stream.avail_in = fread(ibuf, 1, CHUNK, input);

            if (ferror(input)) {
                (void)deflateEnd(&stream);
                return Z_ERRNO;
            }

            flush = feof(input)
                    ? Z_FINISH
                    : Z_NO_FLUSH;

            stream.next_in = ibuf;
            do {
                stream.avail_out = CHUNK;
                stream.next_out = (Byte *)obuf;
                ret = deflate(&stream, flush);
                if (Z_STREAM_ERROR == ret)
                    return ret;


                have_bytes = CHUNK - stream.avail_out;
                if ((fwrite(obuf, 1, have_bytes, output) != have_bytes) || ferror(output)) {
                    (void)deflateEnd(&stream);
                    return Z_ERRNO;
                }
            } while (0 == stream.avail_out);
            assert(0 == stream.avail_in);
        } while (Z_FINISH != flush);
        this->zfini(&stream, 0xDE);
        fclose(output);
        fclose(input);
        return Z_STREAM_END == ret
               ? Z_OK
               : Z_DATA_ERROR;
    } else {
        fclose(output);
        fclose(input);
    }
    this->zfini(&stream, 0xDE);
    return EXIT_FAILURE;
}

int ZHandle::zscan(string binFile)
{
    long int i = 0;
    long int le = 0;
    int matches = 0;

    FILE *this_file = fopen(binFile.c_str(), "rb");

    if (this_file) {
        fseek(this_file, 0, SEEK_END);
        le = ftell(this_file);
        fclose(this_file);
    }

    Byte byte = (Byte)0x00;
    Byte zmagic[2] = { CMF, FLG(9) };

    FILE *bin = fopen(binFile.c_str(), "rb");
    for (; i < le; i++) {
        byte = fgetc(bin);
        if (byte == zmagic[0]) {
            byte = fgetc(bin);
            if (byte == zmagic[1]) {
                (this->ztry(binFile, le, i))
                ? (matches++)
                : (0);
                fseek(bin, i, SEEK_SET);
            } else {
                fseek(bin, i, SEEK_SET);
            }
        }
    }
    fflush(bin);
    fclose(bin);
    return matches;
}

bool ZHandle::ztry(string binary, long int size, long int offset)
{
    Byte buf = 0x00;
    long int iter = (size - offset);
    string rgba = to_string(offset - 1).append(".rgba");
    string zblob = to_string(offset - 1).append(".zlib");
    FILE *stream = fopen(zblob.c_str(), "wb+");
    FILE *binfile = fopen(binary.c_str(), "rb");

    fseek(binfile, offset - 1, SEEK_SET);
    {
        for (; iter != 0; iter--) {
            buf = fgetc(binfile);
            fputc(buf, stream);
        }
    }
    fclose(binfile);
    fclose(stream);

    this->zinflate(zblob, rgba);
    unlink(zblob.c_str());
    if (0 == access(rgba.c_str(), F_OK)) {
        return true;
    } else {
        unlink(rgba.c_str());
        return false;
    }
}

void ZHandle::zini(z_stream *stream, unsigned int mode)
{
    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = Z_NULL;

    switch (mode) {
    case 0x1F:
        stream->avail_in = Z_NULL;
        stream->next_in = Z_NULL;
        inflateInit(stream);
        break;

    case 0xDE:
        deflateInit(stream, ZLIB_BEST);
        break;

    default:
        exit(EXIT_FAILURE);
    }
    return;
}

void ZHandle::zfini(z_stream *stream, unsigned int mode)
{
    switch (mode) {
    case 0xDE:
        (void)deflateEnd(stream);
        break;

    case 0x1F:
        (void)inflateEnd(stream);
        break;
    }
    return;
}

#if 0

int main(int argc, char **argv)
{
    argc--;
    argv++;
    ZHandle Converter;
    if (1 == argc) {
        return Converter.zscan(argv[0]);
    } else if (3 == argc) {
        if (0 == strcmp("inflate", *argv))
            return Converter.zinflate(argv[1], argv[2]);
        else if (0 == strcmp("deflate", *argv))
            return Converter.zdeflate(argv[1], argv[2]);
        else
            return EXIT_FAILURE;
    } else {
        return EXIT_FAILURE;
    }
}

#endif
