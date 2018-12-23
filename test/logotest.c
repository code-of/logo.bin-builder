#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define invalid(f, c) \
    fprintf(stderr, "\x1b[38;2;255;0;50m " f " is invalid !\x1b(B\x1b[m\n", c)
#define valid(f, c) \
    fprintf(stderr, "\x1b[38;2;0;255;50m " f " is valid !\x1b(B\x1b[m\n", c)

int main(int argc, char **argv)
{
    argc--;
    argv++;
    if (argc) {
        int test = EXIT_SUCCESS;
        int j = 0, i = 12;
        char magic[4] = { 0x88, 0x16, 0x88, 0x58 };
        char LOGO[4] = { 'L', 'O', 'G', 'O' };
        char logo[4] = { 'l', 'o', 'g', 'o' };
        char buf[i];
        memset(buf, 0, i);
        FILE *f = fopen(*argv, "r");
        if (fread(buf, sizeof(char), i, f) != i)
            perror("cannot read file !\n");
        fclose(f);
        fprintf(stderr, "magic number test:\n");
        for (i = 0; i < 4; i++) {
            if (buf[i] == magic[i]) {
                valid("%.2X", buf[i]);
            } else {
                invalid("%.2X", buf[i]);
                test = EXIT_FAILURE;
            }
        }
        fprintf(stderr, "logo signature test:\n");
        for (i = 8; i < 12; i++, j++) {
            if (buf[i] == LOGO[j]) {
                valid("%c", buf[i]);
            } else if (buf[i] == logo[j]) {
                valid("%c", buf[i]);
            } else {
                invalid("%c", buf[i]);
                test = EXIT_FAILURE;
            }
        }
        fprintf(stderr, "Result: \x1b[38;2;%i;%i;50m %s is %s !\n",
                (test == EXIT_SUCCESS ? 0 : 255),
                (test == EXIT_SUCCESS ? 255 : 0),
                basename(*argv),
                (test == EXIT_SUCCESS ? "valid" : "invalid"));
        return test;
    }
    return 1;
}
