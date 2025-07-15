#ifndef IFOR_FONT_H
#define IFOR_FONT_H

#include <GLES3/gl3.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "renderer.h"

typedef struct {
    float ax; // advance.x
    float ay; // advance.y

    float bw; // bitmap width
    float bh; // bitmap height

    float bl; // bitmap_left
    float bt; // bitmap_top

    float tx; // offset of glyph in texture coordinates
} CharacterInfo;

typedef struct {
    FT_UInt width;
    FT_UInt height;
    CharacterInfo metrics[128];
} Atlas;

int freetype_init(FT_Library *library, FT_Face *face);
int freetype_create_texture_atlas(Atlas *atlas, FT_Face face);
void freetype_render_text(Atlas *atlas, Renderer *renderer, const char *text,
                          size_t text_size, Vec2f position, Vec4f color);
void freetype_cleanup(FT_Library library, FT_Face face);

#endif /* end of include guard: IFOR_FONT_H */
