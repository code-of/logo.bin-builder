#/* !magick.cc */
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
#include <string>
#include <iostream>
#include <Magick++.h>
#include <converter.hh>

using namespace std;
using namespace Magick;

Converter::Converter(void)
{
    InitializeMagick(NULL);
}

void Converter::rgba_to_png(string fpath, int width, int height)
{
    string size = to_string(width);

    size.append("x");
    size.append(to_string(height));

    Image image;

    try {
        image.size(size.c_str());
        image.depth(8);
        image.read(fpath.c_str());
        image.magick("PNG");
        image.write(fpath.replace(fpath.end() - 4, fpath.end(), "png").c_str());
    } catch (Exception &e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void Converter::png_to_rgba(string fpath)
{
    Image image;

    try {
        image.read(fpath.c_str());
        image.magick("RGBA");
        image.write(fpath.replace(fpath.end() - 3, fpath.end(), "rgba").c_str());
    } catch (Exception &e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}
