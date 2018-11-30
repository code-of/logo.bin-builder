// Copyright MIT License (c) 2018
// Marcel Bobolz <ergotamin@e-p-s.org>

#include "mtkutil.h"

static const unsigned char mtk_magic_numbers[12] =
{
    0x88, 0x16, 0x88, 0x58,
    0x6D, 0x78, 0x13, 0x00,
    0x6C, 0x6F, 0x67, 0x6F
};

static const unsigned char zlib_magic_numbers[2] =
{
    0x78, 0xDA
};

static const unsigned char png_magic_numbers[2] =
{
    0x89, 0x50
};

bool compare_bytes(unsigned char *need, unsigned char *have);
bool is_png_image(char *fname);
bool is_zlib_deflated_file(char *fname);
bool is_mtk_logo_bin(char *fname);

bool compare_bytes(unsigned char *need, unsigned char *have)
{
    bool result = TRUE;
    long le = strlen((const char *)need);

    for (int i = 0; i < le; i++)
        (have[i] != need[i]) ?
        (result = FALSE) :
        (0);

    return result;
}

bool is_png_image(char *fname)
{
    unsigned char bytes[2] = { };
    FILE *this_file = fopen(fname, "rb");

    for (int i = 0; i < 2; i++)
        bytes[i] = (unsigned char)fgetc(this_file);

    fflush(this_file);
    fclose(this_file);

    if (compare_bytes((unsigned char *)png_magic_numbers, (unsigned char *)bytes)) {
        return TRUE;
    } else {
        cerror("This is no valid `PNG´ image file !");
        return FALSE;
    }
}

bool is_zlib_deflated_file(char *fname)
{
    unsigned char bytes[2] = { };
    FILE *this_file = fopen(fname, "rb");

    for (int i = 0; i < 2; i++)
        bytes[i] = (unsigned char)fgetc(this_file);

    fflush(this_file);
    fclose(this_file);

    if (compare_bytes((unsigned char *)zlib_magic_numbers, (unsigned char *)bytes)) {
        return TRUE;
    } else {
        cerror("This is no valid `ZLIB´ deflated file !");
        return FALSE;
    }
}

bool is_mtk_logo_bin(char *fname)
{
    unsigned char bytes[12] = { };
    FILE *this_file = fopen(fname, "rb");

    for (int i = 0; i < 12; i++)
        bytes[i] = (unsigned char)fgetc(this_file);

    fflush(this_file);
    fclose(this_file);

    if (compare_bytes((unsigned char *)mtk_magic_numbers, (unsigned char *)bytes)) {
        return TRUE;
    } else {
        cerror("This is no valid `MTK´ logo.bin file !");
        return FALSE;
    }
}
