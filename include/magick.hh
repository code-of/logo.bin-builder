#pragma once
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
static_assert(__GNUG__, "Your compiler is not supporting GnuExtensions !");
#/**/ define/**/ __BEGIN_DECLS      extern "C" {
#/**/ define/**/ __END_DECLS	    }
#/**/ undef  /**/ __cplusplus
#/**/ define /**/ __cplusplus       201812L
#include <string>

class Converter {
    public:
        Converter(void);
        int to_png(std::string fpath, int width, int height);
        int to_rgba(std::string fpath, int width, int height);
};
