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

constexpr unsigned char MagicHeader[12] = {
    0x88, 0x16, 0x88, 0x58,
    0x6D, 0x78, 0x13, 0x00,
    0x6C, 0x6F, 0x67, 0x6F
};

static const Resolution Resolutions[29] = {
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

    if (true == this->verify(logoBin)) {
        if (false == this->exists(dstDir))
            fs::create_directory(dstDir);
        if (EXIT_SUCCESS == chdir(dstDir.c_str())) {
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

int Builder::pack(string logoFile, string srcDir)
{
    if (true == this->exists(srcDir)) {
        if (EXIT_SUCCESS == chdir(srcDir.c_str())) {
            if (true == this->convert(1)) {
                this->copy(logoFile.string(), ".logo.bak");
                if (true == this->insert(logoFile.string()))
                    return EXIT_SUCCESS;
            }
        }
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

    for (int i = 0; i < 29; i++) {
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

long int Builder::filesize(string fpath)
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

char *Builder::file2bytes(string fpath)
{
    void *bytes;
    long int length = this->filesize(fpath);

    if (0 < length) {
        FILE *file = fopen(fpath.c_str(), "rb");

        if (file) {
            bytes = calloc(length, sizeof(unsigned char));

            if (fread(bytes, 1, length, file)) {
                fclose(file);
                return (char *)bytes;
            } else {
                fclose(file);
                free(bytes);
                perr("Didnt read any bytes from: " << fpath);
                return NULL;
            }
        }
    } else {
        perr("Couldnt open: " << fpath);
        return NULL;
    }
    return NULL;
}

bool Builder::extract(string logoBin)
{
    if (this->zscan(logoBin))
        return true;
    else
        return false;
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

bool Builder::insert(string path)
{
    vector<string> *zblob = new vector<string>;

    for (auto& p: fs::directory_iterator(this->pwd())) {
        string file = p.path().string();

        if (0 == file.substr(file.find_last_of(".") + 1).compare("zlib"))
            zblob->push_back(file);
    }

    if (this->exists(path.c_str())) {
        FILE *logoBin = fopen(path.c_str(), "rb+");

        if (logoBin) {
            for (size_t iter = 0; iter < zblob->size(); iter++) {
                string blob = zblob->at(iter);
                string number = blob.replace(blob.end() - 5, blob.end(), "");
                char *data = this->file2bytes(blob);
                long int offset = strtol(
                    number.substr(number.find_last_of("/") + 1).c_str(),
                    NULL,
                    0
                    );
                fs::remove(blob);

                if (NULL != data) {
                    fseek(logoBin, offset, SEEK_SET);
                    while (0 != *data) {
                        __putc_unlocked_body(*data, logoBin);
                        data++;
                    }
                }
            }
            fclose(logoBin);
            delete zblob;
            return true;
        } else {
            perr("Unable to open file: " << path);
            delete zblob;
            return false;
        }
    } else {
        perr("Cannot find new file: " << path);
        delete zblob;
        return false;
    }
}

bool Builder::convert(int ctx)
{
    vectory<string> *images = new vector<string>;

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
                this->to_png(file, resolution.width, resolution.height);
            fs::remove(file);
        }
        return true;
    }

    if (1 == ctx) {
        for (size_t iter = 0; iter < images->size(); iter++) {
            string file = images->at(iter);
            this->to_rgba(file);
            images->at(iter).assign(
                file.replace(file.end() - 3, file.end(), "rgba").c_str()
                );

            if (EXIT_SUCCESS == this->zdeflate(
                    images->at(iter),
                    file.replace(file.end() - 4, file.end(), "zlib").c_str()
                    )
                )
                fs::remove(images.at(iter));
        }
        delete images;
        return true;
    }
    delete images;
    return false;
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
    if (3 == argc) {
        if (0 == strcmp("unpack", argv[0]))
            return builder.unpack(argv[1], argv[2]);
        if (0 == strcmp("pack", argv[0]))
            return builder.pack(argv[1], argv[2]);
        return EXIT_FAILURE;
    } else {
        return EXIT_FAILURE;
    }
}
