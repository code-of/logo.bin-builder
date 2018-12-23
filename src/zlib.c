// Copyright MIT License (c) 2018
// Marcel Bobolz <ergotamin.source@gmail.com>

#include "mtkutil.h"
#include <zlib.h>
#include <zconf.h>

#define _CHUNKSIZE (0x4000U)

/** @brief zlib - inflating and deflating streams/files.
 *
 * @addtogroup zlib
 * @{
 */

/** @typedef ZlibStream
 * @brief from zlib.h, the struct ZlibStream_s.
*/
typedef z_stream ZlibStream;

/** @typedef byte
 * @brief definition of one byte.
*/
typedef unsigned char byte;

/** @enum ZlibCompression
 * @brief Compression level constants.
 */
enum ZlibCompression {
    ZLIB_DEFAULT	= -1,
    ZLIB_NONE		= 0,
    ZLIB_FAST		= 1,
    ZLIB_BEST		= 9
};

/** @enum ZlibModekey
 * @brief Constants to configure a #ZlibStream .
 */
enum ZlibModeKey {
    DEFLATE		= 9,                /**< deflate */
    INFLATE		= 1,                /**< inflate */
    COMPRESS	= 99,               /**< compress */
    DECOMPRESS	= 1                 /**< decompress */
};

/** @brief zlib magic numbers.
 *
 * @details First 2 Byte at the beginning
 *          of a zlib stream inside a file.
*/
#define Z_CMF()    \
    (              \
        (byte)0x78 \
    )

#define Z_FLG(level)      \
    (                     \
        (level > 1) ?     \
        ((level > 5) ?    \
         ((level < 7) ?   \
          ((byte)0x9C) :  \
          ((byte)0xDA)) : \
         ((byte)0x5E)) :  \
        ((byte)0x01)      \
    )

/* @note prototypes
*/
void zlib_error(int err);
int zlib_stream_new(ZlibStream *Stream, int mode);
int zlib_deflate_FILE(FILE *input, FILE *output);
int zlib_inflate_FILE(FILE *input, FILE *output);
int zlib_try_inflate(char *file, long int size, long int offpos);


/** @brief Configure a Stream.
 * @param[in,out] Stream #ZlibStream
 * @param[in] Mode #int
 * @return #ReturnValue
 */
int zlib_stream_new(ZlibStream *Stream, int mode)
{
    Stream->zalloc = Z_NULL;
    Stream->zfree = Z_NULL;
    Stream->opaque = 0;

    if (mode == DEFLATE) {
        return deflateInit(Stream, ZLIB_BEST);
    } else if (mode == INFLATE) {
        Stream->avail_in = Z_NULL;
        Stream->next_in = Z_NULL;
        return inflateInit(Stream);
    } else {
        return ERROR;
    }
}

/** @brief Evaluate an integer as errorcode.
 * @param[in] err #ReturnValue
 */
void zlib_error(int err)
{
    switch (err) {
    case Z_ERRNO:
        if (ferror(stdin))
            cerror("Error reading stdin ...");
        if (ferror(stdout))
            cerror("Error writing stdout ...");
        break;
    case Z_STREAM_ERROR:
        cerror("Invalid compression Level ...");
        break;
    case Z_DATA_ERROR:
        cerror("Invalid or incomplete deflate Data ...");
        break;
    case Z_MEM_ERROR:
        cerror("Out of memory ...");
        break;
    case Z_VERSION_ERROR:
        cerror("ZLib version mismatch!");
        break;
    }
    return;
}

/** @brief deflate a #FILE
 * @param[in] input #FILE
 * @param[out] output #FILE
 * @return #ReturnValue
 */
int zlib_deflate_FILE(FILE *input, FILE *output)
{
    int ret, flush;
    unsigned int have_data;
    byte in[_CHUNKSIZE];
    byte out[_CHUNKSIZE];
    ZlibStream Stream;

    ret = zlib_stream_new(&Stream, DEFLATE);

    if (ret)
        return ret;

    do {
        Stream.avail_in = fread(in, 1, _CHUNKSIZE, input);

        if (ferror(input)) {
            (void)deflateEnd(&Stream);
            return Z_ERRNO;
        }

        flush = feof(input) ?
                Z_FINISH :
                Z_NO_FLUSH;
        Stream.next_in = in;

        do {
            Stream.avail_out = _CHUNKSIZE;
            Stream.next_out = out;
            ret = deflate(&Stream, flush);
            (ret != Z_STREAM_ERROR) ?
            (nop()) :
            (zlib_error(ret));
            have_data = _CHUNKSIZE - Stream.avail_out;

            if ((fwrite(out, 1, have_data, output) != have_data) || ferror(output)) {
                (void)deflateEnd(&Stream);
                return Z_ERRNO;
            }
        } while (Stream.avail_out == 0);

        assert(Stream.avail_in == 0);
    } while (flush != Z_FINISH);

    (void)deflateEnd(&Stream);
    return ret == Z_STREAM_END ?
           Z_OK :
           Z_DATA_ERROR;
}

/** @brief inflate a #FILE
 * @param[in] input #FILE
 * @param[out] output #FILE
 * @return #ReturnValue
 */
int zlib_inflate_FILE(FILE *input, FILE *output)
{
    int ret;
    unsigned int have_data;
    byte in[_CHUNKSIZE];
    byte out[_CHUNKSIZE];
    ZlibStream Stream;

    ret = zlib_stream_new(&Stream, INFLATE);

    if (ret)
        return ret;

    do {
        Stream.avail_in = fread(in, 1, _CHUNKSIZE, input);

        if (ferror(input)) {
            (void)inflateEnd(&Stream);
            return Z_ERRNO;
        }

        if (Stream.avail_in == 0)
            break;

        Stream.next_in = in;

        do {
            Stream.avail_out = _CHUNKSIZE;
            Stream.next_out = out;
            ret = inflate(&Stream, Z_NO_FLUSH);
            (ret != Z_STREAM_ERROR) ?
            (nop()) :
            (zlib_error(ret));

            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&Stream);
                return ret;
            }

            have_data = _CHUNKSIZE - Stream.avail_out;

            if ((fwrite(out, 1, have_data, output) != have_data) || ferror(output)) {
                (void)inflateEnd(&Stream);
                return Z_ERRNO;
            }
        } while (Stream.avail_out == 0);
    } while (ret != Z_STREAM_END);

    (void)inflateEnd(&Stream);
    return ret == Z_STREAM_END ?
           Z_OK :
           Z_DATA_ERROR;
}

/** @brief in-/deflate input to output.
 * @param mode #int
 * @param[in] input a path.
 * @param[out] output a path.
 * @return #ReturnValue
 */
int zlib_convert_file(int mode, char *input, char *output)
{
    FILE *in = fopen(input, "rb");
    FILE *out = fopen(output, "wb+");
    int ret = ERROR;

    switch (mode) {
    case INFLATE:
    {
        ret = zlib_inflate_FILE(in, out);
        break;
    }
    case DEFLATE:
    {
        ret = zlib_deflate_FILE(in, out);
        break;
    }
    }
    fclose(in);
    fclose(out);
    return ret;
}

/** @brief Try to inflate a file at an offset.
 * @param[in|out] file argument.
 * @param[in|out] size argument.
 * @param[in|out] offpos argument.
 * @return #ReturnValue
*/
int zlib_try_inflate(char *file, long int size, long int offpos)
{
    char fname[512];
    long int i = (size - offpos);
    byte buf = 0x00;
    const char *zname = strbuild("%li.zlib", offpos - 1);

    FILE *stream = fopen(zname, "wb+");
    FILE *binfile = fopen(file, "rb");

    snprintf(fname, 512, "%li.raw", offpos - 1);
    fseek(binfile, offpos - 1, SEEK_SET);
    {
        for (; i != 0; i--) {
            buf = fgetc(binfile);
            fputc(buf, stream);
        }
    }
    fclose(binfile);
    fclose(stream);
    zlib_convert_file(INFLATE, (char *)zname, fname);
    unlink(zname);
    if (access(fname, F_OK) == SUCCESS) {
        return SUCCESS;
    } else {
        unlink(fname);
        return FAILURE;
    }
}

/** @brief Scan a file for possible #ZlibStream
 * @param[in|out] fname argument.
 * @return #ReturnValue
*/
int zlib_file_scan(char *fname)
{
    int matchcount = 0;
    long int i = 0;
    long int le = sizeoffile(fname);
    byte ch = (byte)0x00;
    byte zmagic[2] = { Z_CMF(), Z_FLG(9) };
    FILE *bin = fopen(fname, "rb");

    for (; i < le; i++) {
        ch = fgetc(bin);
        if (ch == zmagic[0]) {
            ch = fgetc(bin);
            if (ch == zmagic[1]) {
                (zlib_try_inflate(fname, le, i) == SUCCESS) ?
                (matchcount++) :
                (0);
                fseek(bin, i, SEEK_SET);
            } else {
                fseek(bin, i, SEEK_SET);
            }
        }
    }
    fflush(bin);
    fclose(bin);
    return matchcount;
}

/** @} */
