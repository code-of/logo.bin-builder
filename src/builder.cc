#/* !builder.cc */
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>

#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <cfile.hh>
#include <converter.hh>
#include <zhandle.hh>

using namespace std;
using namespace file;

typedef struct {
    unsigned int	width;
    unsigned int	height;
} Resolution;

static constexpr Resolution Resolutions[45] = {
    { 15,	27	 }, { 27,	36	 }, { 30,	27	 }, { 32,	105	 },
    { 36,	51	 }, { 38,	54	 }, { 45,	139	 }, { 48,	54	 },
    { 57,	64	 }, { 63,	105	 }, { 84,	121	 }, { 102,	1	 },
    { 108,	121	 }, { 135,	24	 }, { 135,	1	 }, { 138,	20	 },
    { 120,	160	 }, { 160,	240	 }, { 163,	29	 }, { 169,	28	 },
    { 218,	51	 }, { 240,	320	 }, { 240,	400	 }, { 304,	52	 },
    { 320,	480	 }, { 320,	570	 }, { 360,	640	 }, { 480,	640	 },
    { 540,	960	 }, { 640,	360	 }, { 480,	800	 }, { 480,	853	 },
    { 600,	800	 }, { 576,	1024 }, { 600,	1024 }, { 720,	1280 },
    { 768,	1024 }, { 864,	1152 }, { 900,	1440 }, { 1024, 1280 },
    { 1050, 1400 }, { 1050, 1680 }, { 1080, 1920 }, { 1200, 1600 },
    { 1200, 1920 },
};

class Builder : public Converter, public ZHandle {
    public:
        int unpack(string logoFile);
        int pack(string srcDir);

    private:
        void extract(string logoBin);
        void convert(int ctx);
};

static void geometry(string fpath, Resolution *resolution);
static void verify(string fpath);
static void insert(void);

int Builder::unpack(string logoFile)
{
    string logoBin = abspath(logoFile);

    verify(logoBin);

    if (true != exists("out"))
        makedir("out");

    changedir("out");
    this->extract(logoBin);
    this->convert(0);
    copyfile(".logo.bak", logoBin);
    return EXIT_SUCCESS;
}

int Builder::pack(string srcDir)
{
    if (true == exists(srcDir)) {
        changedir(srcDir);
        this->convert(1);
        copyfile("logo.bin", ".logo.bak");
        insert();
        return EXIT_SUCCESS;
    } else {
        error("No such directory:") << srcDir << endl;
    }
    return EXIT_FAILURE;
}

void Builder::extract(string logoBin)
{
    if (0 == this->zlib_scan(logoBin, ZLIB_BEST)) {
        error("Couldnt find any zlib streams:") << logoBin << endl;
        exit(EXIT_FAILURE);
    }
}

void Builder::convert(int ctx)
{
    vector<string> images;

    if (0 == ctx) {
        searchdir(".", ".*\\.rgba", &images);
        for (size_t iter = 0; iter < images.size(); iter++) {
            Resolution resolution;
            geometry(images.at(iter), &resolution);

            if (resolution.height && resolution.width)
                this->rgba_to_png(images.at(iter), resolution.width, resolution.height);

            trash(images.at(iter).c_str());
        }
    }

    if (1 == ctx) {
        searchdir(".", ".*\\.png", &images);
        for (size_t iter = 0; iter < images.size(); iter++) {
            string fname = images.at(iter);
            this->png_to_rgba(fname);
            fname.replace(fname.end() - 3, fname.end(), "rgba");
            images.at(iter).assign(fname.c_str());
            fname.replace(fname.end() - 4, fname.end(), "zlib");

            if (this->zlib_deflate(images.at(iter), fname, ZLIB_BEST)) {
                error("Could not deflate image:") << images.at(iter) << endl;
                exit(EXIT_FAILURE);
            }

            trash(images.at(iter).c_str());
        }
    }
}

static void geometry(string fpath, Resolution *resolution)
{
    unsigned int pixels = 0;

    pixels = (unsigned int)(sizefile(fpath) / 4);

    for (int i = 0; i < 45; i++) {
        if (pixels == (Resolutions[i].width * Resolutions[i].height)) {
            resolution->width = Resolutions[i].width;
            resolution->height = Resolutions[i].height;
            return;
        } else {
            resolution->width = 0;
            resolution->height = 0;
        }
    }
    return;
}

static void verify(string fpath)
{
    int n;
    unsigned char header[32];
    File file(fpath.c_str(), "rb");

    if (file.is_open()) {
        for (n = 0; n < 32; n++)
            header[n] = file.getc();
        file.close();
        for (n = 0; n < 32; n++) {
            if (header[n] == 0x6C) {
                if (header[n + 1] == 0x6F)
                    if (header[n + 2] == 0x67)
                        if (header[n + 3] == 0x6F)
                            return;
            }
        }
        error("Couldnt verify :") << fpath << endl;
    } else {
        error("Could not open: ") << fpath << endl;
    }
    exit(EXIT_FAILURE);
}

static void insert(void)
{
    if (exists("logo.bin")) {
        vector<string> zblob;
        searchdir(".", ".*\\.png", &zblob);
        File logoBin("logo.bin", "rb+");

        if (logoBin.is_open()) {
            for (size_t iter = 0; iter < zblob.size(); iter++) {
                char *data = NULL;
                string zname = zblob.at(iter);
                data = (char *)slurpfile(zname);

                trash(zname.c_str());

                zname.replace(zname.end() - 5, zname.end(), "");
                long offset = strtol(basename(zname.c_str()), NULL, 0);
                logoBin.seek(offset);

                for (int p = 0; 0 != data[p]; p++)
                    logoBin.putc(data[p]);

                free((void *)data);
            }
            logoBin.close();
        } else {
            error("Unable to open 'logo.bin'");
            exit(EXIT_FAILURE);
        }
    } else {
        error("No such file: 'logo.bin'");
        exit(EXIT_FAILURE);
    }
}

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
    cout << "Usage:" << endl;
    cout << "\tmtk-logo-builder unpack [FILE]\t- will unpack [FILE] to a directory 'out/'." << endl;
    cout << "\tmtk-logo-builder pack [SRCDIR]\t- will create 'logo.bin' from files in [SRCDIR]." << endl;
    cout << "\t! DO NOT RENAME UNPACKED IMAGES, OR PACKING WILL FAIL !" << endl;
    return EXIT_SUCCESS;
}
