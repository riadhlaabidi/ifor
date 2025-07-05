#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

char *read_file(const char *path)
{
    char *content = NULL;
    FILE *f = NULL;

    f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "Failed to open file \"%s\": %s\n", path,
                strerror(errno));
        goto defer;
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "Failed to seek in file \"%s\": %s\n", path,
                strerror(errno));
        goto defer;
    }

    long size = ftell(f);
    if (size < 0) {
        fprintf(stderr, "Failed to get file position in file \"%s\": %s\n",
                path, strerror(errno));
        goto defer;
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "Failed to seek in file \"%s\": %s\n", path,
                strerror(errno));
        goto defer;
    }

    content = malloc((size + 1) * sizeof(char));
    if (!content) {
        fprintf(stderr,
                "Failed to allocate memory for file buffer \"%s\": %s\n", path,
                strerror(errno));
        goto defer;
    }
    fread(content, size, 1, f);
    if (ferror(f)) {
        fprintf(stderr, "Failed to read file \"%s\": %s\n", path,
                strerror(errno));
    }
    content[size] = '\0';

defer:
    if (f) {
        fclose(f);
    }
    return content;
}
