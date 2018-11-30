// Copyright MIT License (c) 2018
// Marcel Bobolz <ergotamin@e-p-s.org>

#include "mtkutil.h"
#include <MagickWand/MagickWand.h>

MagickWand *new_magickwand(void);
void free_magickwand(MagickWand *wand);
void convert_png_to_raw(File *this_file);
void convert_raw_to_png(File *this_file);
void convert_raw_to_zlib(File *this_file);
extern int zlib_convert_file(int mode, char *input, char *output);

ImageConverter *new_image_converter(void)
{
    static const ImageConverter this_converter = {
        .to_png		= convert_raw_to_png,
        .to_raw		= convert_png_to_raw,
        .to_zlib	= convert_raw_to_zlib
    };

    return (ImageConverter *)&this_converter;
}

MagickWand *new_magickwand(void)
{
    MagickWand *new_wand = NULL;

    MagickWandGenesis();
    (IsMagickWandInstantiated() == MagickTrue) ?
    (new_wand = NewMagickWand()) :
    (new_wand = (MagickWand *)NULL);
    if (new_wand != (MagickWand *)NULL) {
        return new_wand;
    } else {
        cerror("new_magickwand() failed.");
        abort();
    }
    return NULL;
}

void free_magickwand(MagickWand *wand)
{
    ClearMagickWand(wand);
    DestroyMagickWand(wand);
    MagickWandTerminus();
    wand = (void *)0;
    return;
}

void convert_png_to_raw(File *this_file)
{
    MagickWand *this_wand = new_magickwand();

    MagickSetSize(this_wand, this_file->screen->width, this_file->screen->height);
    MagickSetFormat(this_wand, "PNG");
    MagickSetDepth(this_wand, 8);
    MagickReadImage(this_wand, this_file->png_name);
    MagickSetFormat(this_wand, "RGBA");
    MagickWriteImage(this_wand, this_file->raw_name);
    free_magickwand(this_wand);
    if (filetest(this_file->raw_name) == VALID) {
        unlink(this_file->png_name);
        return;
    } else {
        cerror("convert_png_to_raw() failed.");
        abort();
    }
}

void convert_raw_to_png(File *this_file)
{
    MagickWand *this_wand = new_magickwand();

    MagickSetSize(this_wand, this_file->screen->width, this_file->screen->height);
    MagickSetFormat(this_wand, "RGBA");
    MagickSetDepth(this_wand, 8);
    MagickReadImage(this_wand, this_file->raw_name);
    MagickSetFormat(this_wand, "PNG");
    MagickWriteImage(this_wand, this_file->png_name);
    free_magickwand(this_wand);
    unlink(this_file->raw_name);
}

void convert_raw_to_zlib(File *this_file)
{
    if (zlib_convert_file(9, this_file->raw_name, this_file->zlib_name) == SUCCESS) {
        unlink(this_file->raw_name);
        return;
    } else {
        cerror("convert_raw_to_zlib() failed .");
        abort();
    }
}
