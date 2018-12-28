#pragma once
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
static_assert(__GNUG__, "Your compiler is not supporting GnuExtensions !");
#/**/ define/**/ __BEGIN_DECLS      extern "C" {
#/**/ define/**/ __END_DECLS        }
#/**/ undef  /**/ __cplusplus
#/**/ define /**/ __cplusplus       201812L

#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstddef>
#include <string>
#include <vector>

#include <zlib.hh>
#include <magick.hh>

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

typedef struct {
    unsigned int	width;
    unsigned int	height;
} Resolution;

enum MagicID {
    MTK_LOGOBIN,
    ZLIB_STREAM,
    PNG_PICTURE,
};

class Builder : public ZHandle, public Converter {
    public:
        int unpack(std::string logoFile, std::string dstDir);
        int pack(std::string logoFile, std::string srcDir);

    protected:
        bool compare(const unsigned char *need, const unsigned char *have);
        void geometry(std::string fpath, Resolution *resolution);
        void copy(std::string dest, std::string src);
        bool verify(MagicID id, std::string fpath);
        long int filesize(std::string fpath);
        char *file2bytes(std::string fpath);
        bool extract(std::string logoBin);
        bool exists(std::string path);
        bool convert(int ctx);
        std::string pwd(void);
        bool insert(void);

    private:
        std::vector<std::string> images;
};
