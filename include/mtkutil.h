// Copyright MIT License (c) 2018
// Marcel Bobolz <ergotamin.source@gmail.com>

#pragma once

#define _GNU_SOURCE

// Include POSIX default headers
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <sys/mount.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/capability.h>
#include <linux/mman.h>
#include <linux/types.h>
#include <linux/limits.h>
#include <asm-generic/types.h>
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <ftw.h>
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <pthread.h>

#define FALSE         (1 < 0)
#define TRUE          (1 > 0)

#define fg(r, g, b)   "\x1b[38:2:"r ":"g ":"b "m"
#define bg(r, g, b)   "\x1b[48:2:"r ":"g ":"b "m"
#define sgr()         "\x1b(B\x1b[m"
#define bold()        "\x1b[1m"
#define smul()        "\x1b[4m"
#define sitm()        "\x1b[3m"
#define el1()         "\x1b[1K"
#define up1()         "\x1b[A"

#define megabyte(n)   ((1UL << 20) * n)
#define kilobyte(n)   ((1UL << 10) * n)
#define sizeofchar    (1UL << 0)
#define sizeofint     (1UL << 2)
#define sizeoflong    (1UL << 3)
#define sizeofptr     (1UL << 3)

#define cprint(format, args ...) \
    (fprintf(stdout, format, ## args))

#define cerror(string, ...)         \
    fprintf(stderr,                 \
            fg("255", "255", "255") \
            bg("255", "0", "40")    \
            "\t>_ ERROR :"          \
            sgr()                   \
            fg("255", "0", "0")     \
            "\t"string ""           \
            sgr()                   \
            "\n", ## __VA_ARGS__)

enum returnvalue {
    VALID		= 0,
    SUCCESS		= 0,
    FAILURE		= 1,
    INVALID		= 2,
    EXCEPTION	= 3,
    ERROR		= -1,
    UNDEF		= -2,
    PFAULT		= -3,
};

typedef unsigned int bool;

typedef struct {
    long	width;
    long	height;
} ImageFormat;

typedef struct {
    char *			zlib_name;
    char *			raw_name;
    char *			png_name;
    long			offset;
    long			rawsize;
    ImageFormat *	screen;
} File;

typedef void (*ConvertFunc)(File *);

typedef struct {
    ConvertFunc to_png;
    ConvertFunc to_raw;
    ConvertFunc to_zlib;
} ImageConverter;

File *new_file_object(void);
void free_file_object(File *obj);
ImageConverter *new_image_converter(void);
bool is_mtk_logo_bin(char *fname);
bool is_png_image(char *fname);
bool is_zlib_deflated_file(char *fname);
bool evaluate_screen_size(File *file);
bool pushd(char *path);
bool popd(void);
char *mkdumpdir(void);
void get_dir_contents(char **buf);
long get_biggest_file_in_dir(void);
bool move_file(char *file, char *dest);
void init_file_object(char *fname, File *obj);
int zlib_file_scan(char *fname);
long int sizeoffile(const char *fname);
char *file2str(const char *fpath);
long int writetoatoff(char *fname, long int offset, char *input);
void cputc(char ch, FILE *filep);
int filetest(const char *fname);
void *alloc(long unsigned int bytes);
int filecopy(char *in, char *out);
void *nop(void);
const char *strbuild(char *format, ...);
