#/* !builder.cc */
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
#include <builder.hh>
/*
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <sys/types.h>
#include <sys/stat.h>
*/
using namespace std;
using namespace zlib;
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

static const Geometry Geometries[12] = {
    { 360,	640	 }, { 375, 667 },
    { 720,	1280 }, { 320, 568 },
    { 320,	534	 }, { 480, 800 },
    { 320,	570	 }, { 540, 960 },
    { 1080, 1920 }, { 480, 854 },
    { 412,	732	 }, { 640, 360 }
};

bool Builder::unpack(string dstDir, string logoFile)
{
    fs::path logoBin = fs::absolute(logoFile.c_str());

    if (true == this->verify(MTK_LOGOBIN, logoBin)) {
        if (!this->checkexist(dstDir))
            fs::create_directory(dstDir);
        if (0 == chdir(dstDir.c_str())) {
            if (this->extract(logoBin)) {
                if (this->evaluate(this->pwd())) {
                    if (this->convert(this->pwd())) {
                        this->copy(".logo.bak", logoBin);
                        return true;
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
    return false;
}

bool Builder::repack(string logoBin, string srcDir)
{
    return true;
}

bool Builder::verify(MagicID id, string path)
{
    bool result = false;
    unsigned char *header;
    ifstream file(path.c_str(), ios::in | ios::binary);

    if (file.is_open()) {
        header = new unsigned char [12];
        file.read((char *)header, static_cast<streamsize>(12));
        file.close();
    } else {
        perr("open(" << path << ") failed...");
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

bool Builder::extract(string path)
{
    if (this->zscan(path))
        return true;
    else
        return false;
}

bool Builder::evaluate(string path)
{
    unsigned int pixels = 0;
    unsigned int rgbasize = 0;

    for (auto& p: fs::directory_iterator(path))
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

bool Builder::convert(string path)
{
    /*
    for (auto& p: fs::directory_iterator(path)) {
        if (p.path().remove_filename().string().compare(".zlib")) {
            fs::remove(p.path());
        } else {
            this->images.push_back({
                p.path().string(),
                p.path().replace_extension(".rgba").string(),
                p.path().replace_extension(".png").string(),
                strtoul(p.path().replace_extension("").c_str(), nullptr, 1)
            });
        }
    }
    */
    return true;
}

bool Builder::insert(string path, unsigned long offset)
{
    return true;
}

string Builder::pwd(void)
{
    return fs::current_path().c_str();
}

bool Builder::checkexist(string path)
{
    const fs::path p = path;
    fs::file_status s = fs::file_status{};

    if (fs::status_known(s) ? fs::exists(s) : fs::exists(p))
        return true;
    else
        return false;
}

void Builder::copy(string dest, string src)
{
    try {
        fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
    } catch (fs::filesystem_error &e) {
        perr("Could not copy " << src << " to " << dest << ":" << e.what());
    }
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

#if 1

int main(int argc, char **argv)
{
    argc--;
    argv++;
    Builder builder;
    if (2 == argc)
        return builder.unpack(argv[1], argv[0]);
    else
        return EXIT_FAILURE;
}

#endif
