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

static const struct MagicHeader {
    const unsigned char mtklogo[12];
    const unsigned char zlib[2];
    const unsigned char png[2];
} MagicNumbers = {
    .mtklogo	={
        0x88, 0x16, 0x88, 0x58,
        0x6D, 0x78, 0x13, 0x00,
        0x6C, 0x6F, 0x67, 0x6F
    },
    .zlib		={
        0x78, 0xDA
    },
    .png		={
        0x89, 0x50
    },
};

static const Resolution Geometries[29] = {
    { 38,	54	 }, { 48,	54	 }, { 135,	24	 },
    { 135,	1	 }, { 120,	160	 }, { 160,	240	 },
    { 240,	320	 }, { 240,	400	 }, { 320,	480	 },
    { 320,	570	 }, { 360,	640	 }, { 480,	640	 },
    { 540,	960	 }, { 640,	360	 }, { 480,	800	 },
    { 480,	854	 }, { 600,	800	 }, { 576,	1024 },
    { 600,	1024 }, { 720,	1280 }, { 768,	1024 },
    { 864,	1152 }, { 900,	1440 }, { 1024, 1280 },
    { 1050, 1400 }, { 1050, 1680 }, { 1080, 1920 },
    { 1200, 1600 }, { 1200, 1920 },
};

int Builder::unpack(string logoFile, string dstDir)
{
    fs::path logoBin = fs::absolute(logoFile.c_str());

    if (true == this->verify(MTK_LOGOBIN, logoBin)) {
        if (false == this->exists(dstDir))
            fs::create_directory(dstDir);
        if (EXIT_SUCCESS == chdir(dstDir.c_str())) {
            if (true == this->extract(logoBin)) {
                if (true == this->sizehint(this->pwd())) {
                    if (true == this->convert()) {
                        this->copy(".logo.bak", logoBin);
                        return EXIT_SUCCESS;
                    } else {
                        perr("Something during image conversion went wrong...");
                    }
                } else {
                    perr("Could not evaluate display geometry...");
                }
            } else {
                perr("Extraction of logo.bin failed...");
            }
        } else {
            perr("chdir() failed...");
        }
    } else {
        perr("Invalid/Corrupted logo.bin-file...")
    }
    return EXIT_FAILURE;
}

int Builder::pack(string logoBin, string srcDir)
{
    return EXIT_SUCCESS;
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

void Builder::copy(string dest, string src)
{
    try {
        fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
    } catch (fs::filesystem_error &e) {
        perr("Could not copy " << src << " to " << dest << ":" << e.what());
    }
}

bool Builder::verify(MagicID id, string fpath)
{
    bool result = false;
    unsigned char *header;
    ifstream file(fpath.c_str(), ios::in | ios::binary);

    if (file.is_open()) {
        header = new unsigned char [12];
        file.read((char *)header, static_cast<streamsize>(12));
        file.close();
    } else {
        perr("open(" << fpath << ") failed...");
        exit(EXIT_FAILURE);
    }
    switch (id) {
    case MTK_LOGOBIN:
        result = this->compare(MagicNumbers.mtklogo, header);
        break;

    case ZLIB_STREAM:
        result = this->compare(MagicNumbers.zlib, header);
        break;

    case PNG_PICTURE:
        result = this->compare(MagicNumbers.png, header);
        break;

    default:
        exit(EXIT_FAILURE);
    }

    delete[] header;
    return result;
}

bool Builder::extract(string logoBin)
{
    if (this->zscan(logoBin))
        return true;
    else
        return false;
}

bool Builder::sizehint(string dir)
{
    unsigned int pixels = 0;
    unsigned int rgbasize = 0;

    for (auto& p: fs::directory_iterator(dir))
        if (rgbasize < fs::file_size(p.path()))
            rgbasize = fs::file_size(p.path());

    pixels = rgbasize / 4;

    for (int i = 0; i < 12; i++)
        if (pixels == (Geometries[i].width * Geometries[i].height))
            this->geometry = Geometries[i];

    if (this->geometry.width && this->geometry.height)
        return true;
    else
        return false;
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

string Builder::pwd(void)
{
    return fs::current_path().c_str();
}

bool Builder::convert(void)
{
    /*
    for (auto& p: fs::directory_iterator(path)) {
        if (p.path().remove_filename().string().compare(".zlib")) {
            fs::remove(p.path());
    */
    return true;
}

bool Builder::insert(void)
{
    return true;
}


#if 1

int main(int argc, char **argv)
{
    argc--;
    argv++;
    Builder builder;
    if (2 == argc)
        return builder.unpack(argv[0], argv[1]);
    else
        return EXIT_FAILURE;
}

#endif
