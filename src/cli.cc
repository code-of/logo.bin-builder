#/* !builder.cc */
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>

#include <cstring>
#include <builder.hh>

int main(int argc, char **argv)
{
    argc--;
    argv++;
    if (2 == argc) {
        Builder builder;
        if (0 == strcmp("unpack", argv[0]))
            return builder.unpack(argv[1]);
        if (0 == strcmp("pack", argv[0]))
            return builder.pack(argv[1]);
    }
    printf("Usage:\n");
    printf("\tmtk-logo-builder unpack [FILE]\t- will unpack [FILE] to a directory 'out/'.\n");
    printf("\tmtk-logo-builder pack [SRCDIR]\t- will create 'logo.bin' from files in [SRCDIR].\n");
    printf("\t! DO NOT RENAME UNPACKED IMAGES, OR PACKING WILL FAIL !\n");
    return 0;
}
