#/* !cfile.cc */
#// (c) 2019 MIT License
#// Marcel Bobolz
#// <ergotamin.source@gmail.com>

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <regex>
#include <ftw.h>
#include <fcntl.h>
#include <unistd.h>
#include <cfile.hh>

using namespace std;

namespace file {
    File::File(void)
    {
        this->file = NULL;
    }

    File::File(string path, string mode)
    {
        this->file = NULL;
        this->file = fopen(path.c_str(), mode.c_str());
    }

    void File::open(string path, string mode)
    {
        this->file = fopen(path.c_str(), mode.c_str());
    }

    bool File::is_open(void)
    {
        return NULL != this->file;
    }

    bool File::eof(void)
    {
        return feof(this->file);
    }

    bool File::eof_unlocked(void)
    {
        return __feof_unlocked_body(this->file);
    }

    bool File::has_error(void)
    {
        return ferror(this->file);
    }

    bool File::has_error_unlocked(void)
    {
        return __ferror_unlocked_body(this->file);
    }

    int File::putc(int c)
    {
        return fputc(c, this->file);
    }

    int File::putc_unlocked(int _c)
    {
        return (int)__putc_unlocked_body(_c, this->file);
    }

    int File::getc(void)
    {
        return fgetc(this->file);
    }

    int File::getc_unlocked(void)
    {
        return (int)__getc_unlocked_body(this->file);
    }

    long File::read(void *dest, long length)
    {
        return (long)fread(dest, sizeof(unsigned char), length, this->file);
    }

    long File::write(const void *data, long length)
    {
        return (long)fwrite(data, sizeof(unsigned char), length, this->file);
    }

    long File::size(void)
    {
        fseek(this->file, 0, SEEK_END);
        long length = ftell(this->file);
        fseek(this->file, 0, SEEK_SET);
        return length;
    }

    long File::tell(void)
    {
        return ftell(this->file);
    }

    long File::seek(long offset)
    {
        return (long)fseek(this->file, offset, SEEK_SET);
    }

    void File::close(void)
    {
        fflush(this->file);
        fclose(this->file);
    }

    ostream& error(string what)
    {
        return cerr << "\x1b[38:2:255:20:60m" << what << "\x1b(B\x1b[m" << endl;
    }

    void trash(string path)
    {
        if (!unlink(path.c_str()))
            return;
        error("Could not delete file:") << path << endl;
        exit(EXIT_FAILURE);
    }

    bool exists(string path)
    {
        return !access(path.c_str(), F_OK);
    }

    long sizefile(string fpath)
    {
        File file(fpath, "rb");

        if (file.is_open()) {
            long length = file.size();
            file.close();
            return length;
        } else {
            error("Couldnt open:") << fpath << endl;
            exit(EXIT_FAILURE);
        }
    }

    void *slurpfile(string fpath)
    {
        void *buf = NULL;

        File file(fpath, "rb");

        if (file.is_open()) {
            buf = calloc(file.size(), sizeof(unsigned char));

            if (file.read(buf, file.size())) {
                file.close();
                if (NULL != buf)
                    return buf;
            } else {
                file.close();
                free(buf);
                error("Didnt read any bytes from:") << fpath << endl;
            }
        } else {
            error("Couldnt open:") << fpath << endl;
        }
        exit(EXIT_FAILURE);
    }


    void copyfile(string dest, string src)
    {
        File in(src, "rb");
        File out(dest, "wb+");

        if (in.is_open() && out.is_open()) {
            int c;
            long le = in.size();
            char *buf = (char *)calloc(le, sizeof(unsigned char));
            for (c = 0; c < le; c++)
                buf[c] = in.getc();
            in.close();
            for (c = 0; c < le; c++)
                out.putc(buf[c]);
            out.close();
            free((void *)buf);
            return;
        } else {
            error("Could not copy file:") << "from: " << src << endl << "to: " << dest << endl;
            exit(EXIT_FAILURE);
        }
    }

    void changedir(string path)
    {
        if (!chdir(path.c_str()))
            return;
        error("Could not change directory to:") << path << endl;
        exit(EXIT_FAILURE);
    }

    void makedir(string path)
    {
        if (!mkdir(path.c_str(), 0755))
            return;
        error("Could not create directory at:") << path << endl;
        exit(EXIT_FAILURE);
    }

    string abspath(string path)
    {
        return string(realpath(path.c_str(), NULL));
    }

    string thisdir(void)
    {
        return string(getcwd(NULL, 0));
    }

    static vector<string> *_files;

    static void *_expr;

    int _fmatch(const char *fpath, const struct stat *st, int flag, struct FTW *buf)
    {
        switch (flag) {
        case FTW_F:
        case FTW_D:
        case FTW_SL:
            if (NULL != _expr) {
                regex expr((char *)_expr);
                cmatch match;
                if (regex_match(fpath, match, expr))
                    _files->push_back(fpath);
            } else {
                _files->push_back(fpath);
            }
            break;

        case FTW_DNR:
        case FTW_DP:
        case FTW_SLN:
        default:
            error("Directory search failed !");
            return 1;
        }
        return 0;
    }

    void searchdir(string path, const char *expr, vector<string> *files)
    {
        int fdmax = getdtablesize();

        _expr = (void *)expr;
        _files = files;

        if (!nftw(path.c_str(), _fmatch, fdmax, FTW_PHYS)) {
            return;
        } else {
            error("Could not iterate over directory:") << path << endl;
            exit(EXIT_FAILURE);
        }
    }
}
