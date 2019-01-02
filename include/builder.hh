#pragma once
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
static_assert(__GNUG__, "Your compiler is not supporting GnuExtensions !");

#include <magick.hh>
#include <zlib.hh>

using namespace zlib;
using namespace magick;

class Builder : public Converter, public Handle {
    public:
        int unpack(std::string logoFile);
        int pack(std::string srcDir);

    private:
        void extract(std::string logoBin);
        void convert(int ctx);
};
