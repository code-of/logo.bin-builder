#/* !builder.cc */
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
#include <builder.hh>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <sys/types.h>
#include <sys/stat.h>

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
    { 360,	640	 }, { 375, 667 }, { 720, 1280 }, { 320, 568 },
    { 320,	534	 }, { 480, 800 }, { 320, 570  }, { 540, 960 },
    { 1080, 1920 }, { 480, 854 }, { 412, 732  }, { 640, 360 }
};

Builder::Builder(void)
{
    return;
}

Builder::~Builder(void)
{
    return;
}

bool Builder::unpack(string dstDir, string logoBin)
{
    if (true == this->verify(MTK_LOGOBIN, logoBin)) {
        if (!this->checkexist(dstDir))
            filesystem::create_directory(dstDir);
        filesystem::copy_file(logoBin, dstDir.append("logo.bin"));
        if (0 == chdir(dstDir.c_str())) {
            if (0 < this->extract("logo.bin")) {
                if (this->evaluate(filesystem::current_path().c_str())) {
                    if (this->convert(filesystem::current_path().c_str())) {
                        for (int iter = 0; iter < images.size(); iter++)
                            cout << "Unpacked: " << images.at(iter) << endl;
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
    ;
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
        exit EXIT_FAILURE;
    }
    switch (id) {
    case MTK_LOGOBIN:
        result = this->compare(MagicNumbers.mtklogo, header);
        break;

    case ZLIB_STREAM:
        result = this->compare(MagicNumbers.zlib, header);
        break;

    case PNG_PICTURE;
        result = this->compare(MagicNumbers.png, header);
        break;

    default:
        exit(EXIT_FAILURE);
    }

    delete[] header;
    return result;
}

int Builder::extract(string path)
{
    return this->zscan(path);
}

bool Builder::evaluate(string path)
{
    unsigned int pixels = 0;
    unsigned int rgbasize = 0;

    for (auto& p: filesystem::directory_iterator(path.c_str()))
        if (size < filesystem::file_size(p.path().c_str()))
            rgbasize = filesystem::file_size(p.path().c_str());

    pixels = rgbasize / 4;

    for (int i = 0; i < 12; i++)
        if (pixels == (Geometries[i].width * Geometries[i].height))
            this->geometry = (Geometry *)&Geometries[i];

    if (this->geometry.width && this->geometry.height)
        return true;
    else
        return false;
}

bool Builder::insert(string path, unsigned long offset)
{
    ;
    return true;
}

bool Builder::checkexist(const filesystem::path& p, filesystem::file_status s = filesystem::file_status{})
{
    if (filesystem::status_known(s) ? filesystem::exists(s) : filesystem::exists(p))
        return true;
    else
        return false;
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

static void Builder::copy(string dest, string src)
{
    ;
    ;
}
static void Builder::move(string dest, string src)
{
}
