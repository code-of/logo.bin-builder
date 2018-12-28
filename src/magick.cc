#/* !ffmpeg.cc */
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
#include <Magick++.h>
#include <iostream>
#include <magick.hh>
using namespace std;
using namespace Magick;

Converter::Converter(void)
{
    InitializeMagick(NULL);
}

int Converter::to_png(string fpath, int width, int height)
{
    Image image;

    image.size(to_string(width).append("x").append(to_string(height)).c_str());

    image.depth(8);
    try {
        image.read(fpath.c_str());
        image.magick("PNG");
        image.write(fpath.replace(fpath.end() - 4, fpath.end(), "png").c_str());
    } catch (Exception &e) {
        cerr << "Caught an exception: " << e.what() << endl;
    }
    return EXIT_SUCCESS;
}

int Converter::to_rgba(string fpath, int width, int height)
{
    Image image;

    image.size(to_string(width).append("x").append(to_string(height)).c_str());

    image.depth(8);
    try {
        image.read(fpath.c_str());
        image.magick("RGBA");
        image.write(fpath.replace(fpath.end() - 3, fpath.end(), "rgba").c_str());
    } catch (Exception &e) {
        cerr << "Caught an exception: " << e.what() << endl;
    }
    return EXIT_SUCCESS;
}
