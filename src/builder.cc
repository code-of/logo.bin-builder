#/* !builder.cc */
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
#include <builder.hh>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>
#include <new>

using namespace std;
namespace fs = std::filesystem;

static constexpr unsigned char MagicHeader[12] = {
    0x88, 0x16, 0x88, 0x58,
    0x6D, 0x78, 0x13, 0x00,
    0x6C, 0x6F, 0x67, 0x6F
};

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

int Builder::unpack(string logoFile)
{
    fs::path logoBin = fs::absolute(logoFile.c_str());

    if (true == this->verify(logoBin)) {
        if (false == this->exists("out"))
            fs::create_directory("out");
        if (EXIT_SUCCESS == chdir("out")) {
            if (true == this->extract(logoBin)) {
                if (true == this->convert(0)) {
                    this->copy(".logo.bak", logoBin);
                    return EXIT_SUCCESS;
                }
            }
        }
    } else {
        perr("Invalid/Corrupted logo.bin-file...")
    }
    return EXIT_FAILURE;
}

int Builder::pack(string srcDir)
{
    if (true == this->exists(srcDir)) {
        if (EXIT_SUCCESS == chdir(srcDir.c_str())) {
            if (true == this->convert(1)) {
                this->copy("logo.bin", ".logo.bak");
                if (true == this->insert())
                    return EXIT_SUCCESS;
            }
        }
    } else {
        perr("No such directory: " << srcDir);
    }
    return EXIT_FAILURE;
}

bool Builder::compare(unsigned const char *need, unsigned const char *have)
{
    bool result = true;
    long int length = strlen((const char *)need);

    for (int i = 0; i < length; i++)
        (have[i] != need[i])
        ? (result = false)
        : (0);

    return result;
}

void Builder::geometry(string fpath, Resolution *resolution)
{
    unsigned int pixels = 0;
    fs::path path;

    path.assign(fpath);
    pixels = fs::file_size(path) / 4;

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

void Builder::copy(string dest, string src)
{
    try {
        fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
    } catch (fs::filesystem_error &e) {
        perr("Could not copy " << src << " to " << dest << ":" << e.what());
    }
}

long int Builder::fsize(string fpath)
{
    long int length = 0;
    FILE *file = fopen(fpath.c_str(), "rb");

    if (file) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fclose(file);
        return length;
    } else {
        perr("Couldnt open: " << fpath);
        return 0L;
    }
}

bool Builder::extract(string logoBin)
{
    if (this->zlib_scan(logoBin, ZLIB_BEST))
        return true;
    else
        return false;
}

void *Builder::ftobuf(string fpath)
{
    void *buf;
    long int length = this->fsize(fpath);

    if (0 < length) {
        FILE *file = fopen(fpath.c_str(), "rb");

        if (file) {
            buf = calloc(length, sizeof(unsigned char));

            if (fread(buf, 1, length, file)) {
                fclose(file);
                return buf;
            } else {
                fclose(file);
                free(buf);
                perr("Didnt read any bytes from: " << fpath);
            }
        } else {
            perr("Couldnt open: " << fpath);
        }
    } else {
        perr("Size of File equal 0: " << fpath);
    }
    return NULL;
}


bool Builder::verify(string fpath)
{
    bool result = false;
    unsigned char *header;
    ifstream file(fpath.c_str(), ios::in | ios::binary);

    if (file.is_open()) {
        header = new unsigned char [12];
        file.read((char *)header, static_cast<streamsize>(12));
        file.close();
    } else {
        perr("Could not open: " << fpath);
        exit(EXIT_FAILURE);
    }
    result = this->compare(MagicHeader, header);
    delete[] header;
    return result;
}

bool Builder::exists(string path)
{
    const fs::path p = path;
    fs::file_status s = fs::file_status{};

    if (fs::status_known(s) ? fs::exists(s) : fs::exists(p))
        return true;
    else
        return false;
}

bool Builder::convert(int ctx)
{
    vector<string> *images = new vector<string>;

    for (auto& p: fs::directory_iterator(this->pwd())) {
        string file = p.path().string();

        if (0 == ctx)
            if (0 == file.substr(file.find_last_of(".") + 1).compare("rgba"))
                images->push_back(file);

        if (1 == ctx)
            if (0 == file.substr(file.find_last_of(".") + 1).compare("png"))
                images->push_back(file);
    }

    if (0 == ctx) {
        for (size_t iter = 0; iter < images->size(); iter++) {
            Resolution resolution;
            string file = images->at(iter);
            this->geometry(file, &resolution);

            if (resolution.height && resolution.width)
                this->rgba_to_png(file, resolution.width, resolution.height);

            fs::remove(file);
        }
        return true;
    }

    if (1 == ctx) {
        for (size_t iter = 0; iter < images->size(); iter++) {
            string file = images->at(iter);
            this->png_to_rgba(file);
            file.replace(file.end() - 3, file.end(), "rgba");
            images->at(iter).assign(file.c_str());
            file.replace(file.end() - 4, file.end(), "zlib");

            if (EXIT_SUCCESS != this->zlib_deflate(images->at(iter), file, ZLIB_BEST))
                perr("Could not deflate image: " << images->at(iter));

            fs::remove(images->at(iter));
        }
        delete images;
        return true;
    }
    delete images;
    return false;
}

bool Builder::insert(void)
{
    vector<string> *zblob = new vector<string>;

    for (auto& p: fs::directory_iterator(this->pwd())) {
        string file = p.path().string();

        if (0 == file.substr(file.find_last_of(".") + 1).compare("zlib"))
            zblob->push_back(file);
    }

    if (this->exists("logo.bin")) {
        FILE *logoBin = fopen("logo.bin", "rb+");

        if (logoBin) {
            for (size_t iter = 0; iter < zblob->size(); iter++) {
                char *data = NULL;
                string zname = zblob->at(iter);
                data = (char *)this->ftobuf(zname);

                if (NULL != data) {
                    fs::remove(zname);
                    zname.replace(zname.end() - 5, zname.end(), "");
                    long offset = strtol(
                        zname.substr(zname.find_last_of("/") + 1).c_str(),
                        NULL, 0);
                    fseek(logoBin, offset, SEEK_SET);

                    for (int p = 0; 0 != data[p]; p++)
                        __putc_unlocked_body(data[p], logoBin);

                    free((void *)data);
                } else {
                    perr("Didnt read any bytes...");
                    fclose(logoBin);
                    delete zblob;
                    return false;
                }
            }
            fclose(logoBin);
            delete zblob;
            return true;
        } else {
            perr("Unable to open 'logo.bin'");
            delete zblob;
            return false;
        }
    } else {
        perr("Cannot find 'logo.bin'");
        delete zblob;
        return false;
    }
}

string Builder::pwd(void)
{
    return fs::current_path().c_str();
}

int main(int argc, char **argv)
{
    argc--;
    argv++;
    Builder builder;
    if (2 == argc) {
        if (0 == strcmp("unpack", argv[0]))
            return builder.unpack(argv[1]);
        if (0 == strcmp("pack", argv[0]))
            return builder.pack(argv[1]);
    }
    pout("Usage:");
    pout("\tmtk-logo-builder unpack [FILE]\t- will unpack [FILE] to a directory 'out/'.");
    pout("\tmtk-logo-builder pack [SRCDIR]\t- will create 'logo.bin' from files in [SRCDIR].");
    pout("\t! DO NOT RENAME UNPACKED IMAGES, OR PACKING WILL FAIL !");
    return EXIT_SUCCESS;
}
