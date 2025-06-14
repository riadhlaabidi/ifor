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
        goto fail;
    }

    if (fseek(f, 0, SEEK_END) < 0) {
        fprintf(stderr, "Failed to seek in file \"%s\": %s\n", path,
                strerror(errno));
        goto fail;
    }

    long size = ftell(f);
    if (size < 0) {
        fprintf(stderr, "Failed to get file position in file \"%s\": %s\n",
                path, strerror(errno));
        goto fail;
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        fprintf(stderr, "Failed to seek in file \"%s\": %s\n", path,
                strerror(errno));
        goto fail;
    }

    content = malloc(size * sizeof(char));
    if (!content) {
        fprintf(stderr,
                "Failed to allocate memory for file buffer \"%s\": %s\n", path,
                strerror(errno));
        goto fail;
    }
    fread(content, size, 1, f);
    if (ferror(f)) {
        fprintf(stderr, "Failed to read file \"%s\": %s\n", path,
                strerror(errno));
    }

fail:
    if (f) {
        fclose(f);
    }
    return content;
}
