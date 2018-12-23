// Copyright MIT License (c) 2018
// Marcel Bobolz <ergotamin.source@gmail.com>

#include "mtkutil.h"

#define UNPACK_DESTDIR "extracted_logo_bin"
#define REPACK_CONFIG  ".do_not_delete"
#define LOGO_BIN_BLOB  ".do_not_touch"

int unpack_logo_bin(char *lname);
int repack_logo_dir(char *dirname);
extern int zlib_file_scan(char *fname);

int main(int argc, char **arg)
{
    argc--;
    arg++;
    if (argc > 1) {
        if (strcmp(*arg, "unpack") == VALID) {
            arg++;
            if (is_mtk_logo_bin(*arg))
                return unpack_logo_bin(*arg);
        } else if (strcmp(*arg, "repack") == VALID) {
            arg++;
            if (!access(*arg, F_OK | R_OK))
                return repack_logo_dir(*arg);
            cerror("Directory doesn't exist or isn't readable !");
        }
    }
    cprint("\n\tmtkutil unpack 'logo.bin'\n\tmtkutil repack [DIRNAME]\n\n\t"
           "IMPORTANT:\n\t"
           "Do not change filenames or any repack attempt\n\t"
           "will result in a corrupted file...\n");
    return FAILURE;
}

int unpack_logo_bin(char *lname)
{
    if (mkdir(UNPACK_DESTDIR, 0755) != SUCCESS) {
        cerror("mkdir("UNPACK_DESTDIR ") failed.");
        return FAILURE;
    } else {
        if (pushd("./"UNPACK_DESTDIR)) {
            char logo_bin_path[512];
            snprintf(logo_bin_path, 512, "%s%s", "../", lname);
            int exc = zlib_file_scan(logo_bin_path);
            if (exc > 1) {
                cprint("\tExtracted"
                       " %i files ...\n",
                       exc);
                File *this_file = new_file_object();
                long raw_bytes = get_biggest_file_in_dir();
                this_file->rawsize = raw_bytes;
                if (evaluate_screen_size(this_file)) {
                    cprint("\tDisplay geometry found:"
                           " %li x %li" " ...\n",
                           this_file->screen->width,
                           this_file->screen->height
                           );
                    char **files = (char **)alloc(sizeof(char *) * 512);
                    get_dir_contents(files);
                    ImageConverter *convert_file = new_image_converter();
                    for (int i = 0; files[i] != NULL; i++) {
                        File *current_file = new_file_object();
                        init_file_object(files[i], current_file);
                        current_file->screen = this_file->screen;
                        convert_file->to_png(current_file);
                        free_file_object(current_file);
                    }
                    files = NULL;
                    FILE *repack_config = fopen(REPACK_CONFIG, "wb+");
                    fprintf(
                        repack_config,
                        "%li",
                        this_file->rawsize
                        );
                    fflush(repack_config);
                    fclose(repack_config);
                    free_file_object(this_file);
                    filecopy(logo_bin_path, LOGO_BIN_BLOB);
                    cprint("\t<Finished>\n"
                           "\t! IMPORTANT !\n"
                           "Filenames need not to be changed !\n"
                           );
                    if (popd())
                        return SUCCESS;
                    else
                        cerror("popd() failed.");
                } else {
                    cerror("Could not evaluate_screen_size().");
                }
            } else {
                cerror("No files have been extracted.");
            }
        } else {
            cerror("pushd("UNPACK_DESTDIR ") failed.");
        }
    }
    return FAILURE;
}

int repack_logo_dir(char *dirname)
{
    if (pushd(dirname)) {
        if (access(REPACK_CONFIG, F_OK)) {
            cerror("Missing repack config '.do_not_delete' .");
            return FAILURE;
        } else if (access(LOGO_BIN_BLOB, F_OK)) {
            cerror("Missing logo.bin copy '.do_not_touch' .");
            return FAILURE;
        } else {
            File *this_file = new_file_object();
            long size;
            FILE *repack_config = fopen(REPACK_CONFIG, "rb");
            fscanf(repack_config, "%li", &size);
            fclose(repack_config);
            unlink(REPACK_CONFIG);
            if ((this_file->rawsize = size) > 0) {
                filecopy(LOGO_BIN_BLOB, "./../new_logo.bin");
                unlink(LOGO_BIN_BLOB);
                if (evaluate_screen_size(this_file)) {
                    char **files = (char **)alloc(sizeof(char *) * 512);
                    get_dir_contents(files);
                    ImageConverter *convert_file = new_image_converter();
                    for (int i = 0; files[i] != NULL; i++) {
                        File *current_file = new_file_object();
                        init_file_object(files[i], current_file);
                        current_file->screen = this_file->screen;
                        convert_file->to_raw(current_file);
                        convert_file->to_zlib(current_file);
                        if (is_zlib_deflated_file(current_file->zlib_name)) {
                            writetoatoff(
                                "./../new_logo.bin",
                                (long unsigned int)current_file->offset,
                                current_file->zlib_name
                                );
                            unlink(current_file->zlib_name);
                        } else {
                            cerror("Could not verify deflated file.");
                            return FAILURE;
                        }
                        free_file_object(current_file);
                    }
                    files = NULL;
                    free_file_object(this_file);
                    if (popd()) {
                        remove(dirname);
                        if (is_mtk_logo_bin("new_logo.bin")) {
                            cprint("\t<Successful build>\n"
                                   "\t<new_logo.bin>\n");
                            return SUCCESS;
                        } else {
                            cerror("Could not verify 'new_logo.bin' .");
                        }
                    } else {
                        cerror("popd() failed.");
                    }
                } else {
                    cerror("Could not evaluate_screen_size() .");
                }
            } else {
                cerror("Could not read repack_config-> '.do_not_delete'");
            }
            return FAILURE;
        }
    }
    return ERROR;
}
