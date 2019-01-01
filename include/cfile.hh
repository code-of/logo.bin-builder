#pragma once
#// (c) 2018 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>
static_assert(__GNUG__, "Your compiler is not supporting GnuExtensions !");

#include <iostream>
#include <string>
#include <vector>

namespace file {
    class File {
        public:
            File(void);
            File(std::string path, std::string mode);
            void open(std::string path, std::string mode);
            bool is_open(void);
            void close(void);
            long size(void);
            long tell(void);
            long seek(long offset);
            void putc(char c);
            unsigned char getc(void);
            long read(void *dest, long length);

        private:
            FILE *file;
    };

    std::ostream& error(std::string e);

    template<typename T, typename ... Targs>
    std::ostream& error(std::string e, T s, Targs... Vargs);

    void trash(std::string path);

    bool exists(std::string path);

    long sizefile(std::string fpath);

    void *slurpfile(std::string fpath);

    void copyfile(std::string dest, std::string src);

    void changedir(std::string path);

    void makedir(std::string path);

    std::string thisdir(void);

    std::string abspath(std::string path);

    void searchdir(std::string path, const char *regex, std::vector<std::string> *files);
}
