#pragma once
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
static_assert(__GNUG__, "Your compiler is not supporting GnuExtensions !");
#/**/ undef  /**/ __cplusplus
#/**/ define /**/ __cplusplus       202012L
#/**/ define /**/ __Begin           extern "C" {
#/**/ define /**/ __End             }

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <array>
#include <vector>
#include <utility>
#include <thread>
#include <exception>
#include <new>
#include <zlib.hh>

#define __const           __attribute__((const))
#define __flat            __attribute__((flatten))
#define __notnull(...)    __attribute__((nonnull ## __VA_ARGS__)))
#define __section(label)  __attribute__((section(#label)))

#define fg(r, g, b)       "\x1b[38:2:" << r << ":" << g << ":" << b << "m"
#define bg(r, g, b)       "\x1b[48:2:" << r << ":" << g << ":" << b << "m"
#define sgr()             "\x1b(B\x1b[m"
#define bold()            "\x1b[1m"
#define smul()            "\x1b[4m"
#define sitm()            "\x1b[3m"
#define el1()             "\x1b[1K"
#define up1()             "\x1b[A"

#define pout(...) \
    std::cout << __VA_ARGS__ << endl;

#define perr(...) \
    std::cerr << fg("255", "0", "0") << __VA_ARGS__ << sgr() << std::endl;

enum MagicID {
    MTK_LOGOBIN,
    ZLIB_STREAM,
    PNG_PICTURE,
};

typedef struct {
    unsigned int	width;
    unsigned int	height;
} Geometry;

typedef struct {
    std::string		zlibFile;
    std::string		rgbaFile;
    std::string		pngFile;
    unsigned long	offset;
} Image;

class Builder : public zlib::ZConverter {
    public:
        bool unpack(std::string dstDir, std::string logoBin);
        bool repack(std::string logoBin, std::string srcDir);

    protected:
        Geometry geometry;
        std::vector<Image> images;
        bool verify(MagicID id, std::string path);
        bool extract(std::string path);
        bool evaluate(std::string path);
        bool convert(std::string path);
        bool insert(std::string path, unsigned long offset);

    private:
        std::string pwd(void);
        bool checkexist(std::string path);
        void copy(std::string dest, std::string src);
        bool compare(const Byte *need, const Byte *have);
};
