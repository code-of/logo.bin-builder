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
#include <cstddef>
#include <string>
#include <vector>
#include <unistd.h>
#include <zlib.hh>
#include <gmagick.hh>

#define fg(r, g, b)       "\x1b[38:2:" << r << ":" << g << ":" << b << "m"
#define sgr()             "\x1b(B\x1b[m"

#define pout(...) \
    std::cout << __VA_ARGS__ << endl;

#define perr(...) \
    std::cerr << fg("255", "0", "0") << __VA_ARGS__ << sgr() << std::endl;

typedef struct {
    unsigned int	width;
    unsigned int	height;
} Resolution;

class Builder : public ZHandle, public Converter {
    public:
        int unpack(std::string logoFile);
        int pack(std::string srcDir);

    protected:
        bool compare(const unsigned char *need, const unsigned char *have);
        void geometry(std::string fpath, Resolution *resolution);
        void copy(std::string dest, std::string src);
        long int fsize(std::string fpath);
        bool extract(std::string logoBin);
        void *ftobuf(std::string fpath);
        bool verify(std::string fpath);
        bool exists(std::string path);
        bool convert(int ctx);
        bool insert(void);
        std::string pwd(void);
};
