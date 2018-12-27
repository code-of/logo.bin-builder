#pragma once
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
static_assert(__GNUG__, "Your compiler is not supporting GnuExtensions !");
#pragma ident             __BASE_FILE__
#undef __cplusplus
#define __cplusplus       202012L

#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <string>
#include <array>
#include <vector>
#include <utility>
#include <new>
#include <exception>
#include <thread>

#include <zlib.hh>

using namespace std;
using namespace zlib;

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
    cout << __VA_ARGS__ << endl;

#define perr(...) \
    cerr << fg("255", "0", "0") << __VA_ARGS__ << $(sgr) << endl;


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
    string			zlibfile;
    string			rawfile;
    string			pngfile;
    unsigned long	offset;
} Image;

class Builder : public ZConverter {
    public:
        Builder(void);
        ~Builder(void);
        bool unpack(string dstDir, string logoBin);
        bool repack(string logoBin, string srcDir);

    protected:
        vector<Image> images;
        bool verify(MagicID id, string path);
        int extract(string path);
        bool evaluate(string path);
        bool convert(string path);
        bool insert(string path, unsigned long offset);

    private:
        Geometry geometry;
        bool checkexist(const filesystem::path& p, filesystem::file_status s = filesystem::file_status{});
        bool compare(unsigned const char *need, unsigned const char *have);
        void copy(string dest, string src);
        void move(string dest, string src);
};
