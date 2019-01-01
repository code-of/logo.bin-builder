#pragma once
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
static_assert(__GNUG__, "Your compiler is not supporting GnuExtensions !");

#include <string>

class Converter {
    public:
        Converter(void);
        void rgba_to_png(std::string fpath, int width, int height);
        void png_to_rgba(std::string fpath);
};
