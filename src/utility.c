// Copyright MIT License (c) 2018
// Marcel Bobolz <ergotamin@e-p-s.org>

#include "mtkutil.h"

typedef struct _DirStack {
    char	dirs[512][512];
    int		index;
} DirStack;

static DirStack *dir_stack;

static const ImageFormat screen_size[12] = {
    { 360,	640	 }, { 375, 667 }, { 720, 1280 }, { 320, 568 },
    { 320,	534	 }, { 480, 800 }, { 320, 570  }, { 540, 960 },
    { 1080, 1920 }, { 480, 854 }, { 412, 732  }, { 640, 360 }
};

void *nop(void)
{
    return NULL;
}

void *alloc(long unsigned int bytes)
{
    void *mem = (void *)0;

    mem = malloc(sizeofptr * bytes);
    memset(mem, 0, bytes * sizeofptr);
    if (mem != (void *)0) {
        return mem;
    } else {
        cerror("alloc() - memory allocation failed !");
        abort();
    }
}

bool move_file(char *file, char *dest)
{
    char new_path[512] = { };

    snprintf(new_path, (strlen(file) + strlen(dest) + 1), "%s%s", dest, file);
    return !rename(file, new_path);
}

int filetest(const char *fname)
{
    if ((access(fname, F_OK)) != -1)
        return VALID;
    else return INVALID;
}

void get_dir_contents(char **buf)
{
    struct dirent *this_dir;
    DIR *cwd = opendir(".");

    if (cwd) {
        while ((this_dir = readdir(cwd)) != NULL) {
            if (this_dir->d_type == DT_REG) {
                *buf = alloc(512);
                strcpy(*buf, this_dir->d_name);
                buf++;
            }
        }
        closedir(cwd);
        return;
    } else {
        cerror("opendir(\".\") failed.");
        abort();
    }
}

long get_biggest_file_in_dir(void)
{
    char **files = (char **)alloc(sizeof(char *) * 512);
    long this_size = 0;

    get_dir_contents(files);
    while (*files) {
        long size = sizeoffile(*files);
        if (size > this_size)
            this_size = size;
        files++;
    }
    return this_size;
}

bool pushd(char *path)
{
    if (!dir_stack) {
        dir_stack = (DirStack *)alloc(sizeof(DirStack));
        dir_stack->index = 0;
    }
    char *cwd = alloc(512);
    getcwd(cwd, 512);
    if (strlen(cwd) > 1) {
        strncpy(dir_stack->dirs[dir_stack->index++], cwd, strlen(cwd) + 1);
        free(cwd);
        if (chdir(path) == SUCCESS)
            return TRUE;
        else
            return FALSE;
    }
    cerror("Could not retrieve current directory.");
    abort();
}

bool popd(void)
{
    if (!dir_stack) {
        return FALSE;
    } else if ((dir_stack->index - 1) >= 0) {
        dir_stack->index--;
        if (chdir(dir_stack->dirs[dir_stack->index]) == SUCCESS) {
            if ((dir_stack->index - 1) < 0)
                free(dir_stack);
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        free(dir_stack);
        return FALSE;
    }
    cerror("undefined error using popd().");
    abort();
}

char *mkdumpdir(void)
{
    char tmpl[512] = { };

    snprintf(tmpl, 512, "%s", ".logo_bin_dumpXXXXXX");
    if (mkdtemp(tmpl) != NULL) {
        return strdup(tmpl);
    } else {
        cerror("Could not execute mkdtemp().");
        abort();
    }
}

bool evaluate_screen_size(File *file)
{
    long absolute_pixel_count = ((file->rawsize) / 4);

    for (int i = 0; i < 12; i++) {
        ((screen_size[i].width * screen_size[i].height) == absolute_pixel_count) ?
        (file->screen = (ImageFormat *)&screen_size[i]) :
        (nop());
    }

    if (file->screen->width && file->screen->height)
        return TRUE;
    else
        return FALSE;
}

File *new_file_object(void)
{
    File *new_file = (File *)alloc(sizeof(File));

    new_file->zlib_name = alloc(512);
    new_file->raw_name = alloc(512);
    new_file->png_name = alloc(512);
    new_file->offset = (0xFFFFFF) + 1;
    new_file->rawsize = -1;
    new_file->screen = (ImageFormat *)alloc(sizeof(ImageFormat));

    if (new_file != NULL)
        return new_file;
    else
        return NULL;
}

void init_file_object(char *fname, File *obj)
{
    char *this_file = strdup(fname);
    char *tr = strrchr(this_file, '.');

    *tr = 0;

    if (!strlen(this_file))
        return;

    obj->offset = strtol(this_file, (char **)0, 10);
    strcpy(obj->zlib_name, this_file);
    strcat(obj->zlib_name, ".zlib\0");
    strcpy(obj->raw_name, this_file);
    strcat(obj->raw_name, ".raw\0");
    strcpy(obj->png_name, this_file);
    strcat(obj->png_name, ".png\0");
}

void free_file_object(File *obj)
{
    memset(obj->zlib_name, 0, strlen(obj->zlib_name));
    memset(obj->raw_name, 0, strlen(obj->raw_name));
    memset(obj->png_name, 0, strlen(obj->png_name));
    obj->rawsize = -1;
    obj->offset = -1;
    obj = (File *)NULL;
}

void cputc(char ch, FILE *filep)
{
    __putc_unlocked_body(ch, filep);
}

long int sizeoffile(const char *fname)
{
    long int len;
    FILE *this_file = fopen(fname, "rb");

    if (this_file) {
        fseek(this_file, 0, SEEK_END);
        len = ftell(this_file);
        fclose(this_file);
        return len;
    }
    return 0L;
}

int filecopy(char *in, char *out)
{
    unsigned char buff[kilobyte(4)];
    FILE *src = fopen(in, "rb");
    FILE *dst = fopen(out, "wb+");
    long unsigned int nbyte;

    while ((nbyte = fread((char *)buff, sizeofchar, sizeof(buff), src)) > 0)
        if (fwrite((char *)buff, sizeofchar, nbyte, dst) != nbyte)
            return FAILURE;

    fflush(dst);
    fclose(dst);
    fclose(src);
    return SUCCESS;
}

char *file2str(const char *fpath)
{
    char *str;
    long int len = 0;

    FILE *this_file = fopen(fpath, "rb");

    if (this_file) {
        len = sizeoffile(fpath);
        unsigned char buff[sizeofchar * len + 1];

        fread(buff, 1, len, this_file);
        fclose(this_file);
        str = alloc(len + 1);
        strcpy(str, (const char *)buff);

        memset(buff, 0, len + 1);
        return str;
    }
    return NULL;
}

long int writetoatoff(char *fname, long int offset, char *input)
{
    long int len;
    FILE *this_file = fopen(fname, "rb+");

    if (this_file)
        fseek(this_file, offset, SEEK_SET);
    else return FAILURE;

    if (filetest(input) == VALID) {
        len = sizeoffile(input) + 1;
        char *fstring = file2str(input);

        while ((len != 0) && (*fstring)) {
            cputc(*fstring, this_file);
            fstring++;
            len--;
        }
        fflush(this_file);
    } else {
        len = (strlen(input) + 1);

        while (*input != 0) {
            cputc(*input, this_file);
            input++;
        }
        fflush(this_file);
    }
    fclose(this_file);
    return len;
}

const char *strbuild(char *format, ...)
{
    char *buff = alloc(kilobyte(4));

    va_list argp;

    va_start(argp, format);
    vsprintf(buff, format, argp);
    va_end(argp);

    return strdup(buff);
}
