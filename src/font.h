#ifndef IFOR_FONT_H
#define IFOR_FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdint.h>

typedef struct {
    struct character_info {

        float ax; // advance.x
        float ay; // advance.y

        float bw; // bitmap width
        float bh; // bitmap height

        float bl; // bitmap_left
        float bt; // bitmap_top

        float tx; // offset of glyph in texture coordinates
    } c[128];
} Atlas;

int freetype_init(FT_Library *library, FT_Face *face);
// void freetype_render(uint32_t *data, int width, const char *string, int
// length);

// void freetype_cleanup(void);

#endif /* end of include guard: IFOR_FONT_H */
