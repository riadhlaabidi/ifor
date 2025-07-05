#include <GLES3/gl3.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "font.h"
#include "freetype/freetype.h"
#include "la.h"
#include "renderer.h"

int freetype_create_texture_atlas(Atlas *atlas, FT_Face face)
{

    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "[FreeType] Error loading character \'%c\'\n", i);
            return 0;
        }

        atlas->width += face->glyph->bitmap.width;
        if (face->glyph->bitmap.rows > atlas->height) {
            atlas->height = face->glyph->bitmap.rows;
        }
    }

    GLuint texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Create an empty texture for the atlas
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height, 0,
                 GL_RED, GL_UNSIGNED_BYTE, NULL);

    int x = 0;
    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "[FreeType] Error loading character \'%c\'\n", i);
            return 0;
        }

        atlas->metrics[i].ax = face->glyph->advance.x >> 6;
        atlas->metrics[i].ay = face->glyph->advance.y >> 6;
        atlas->metrics[i].bw = face->glyph->bitmap.width;
        atlas->metrics[i].bh = face->glyph->bitmap.rows;
        atlas->metrics[i].bl = face->glyph->bitmap_left;
        atlas->metrics[i].bt = face->glyph->bitmap_top;
        atlas->metrics[i].tx = (float)x / (float)atlas->width;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, face->glyph->bitmap.width,
                        face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE,
                        face->glyph->bitmap.buffer);

        x += face->glyph->bitmap.width;
    }

    return 1;
}

int freetype_init(FT_Library *library, FT_Face *face)
{
    if (FT_Init_FreeType(library)) {
        fprintf(stderr, "[FreeType] Error during library initialization.\n");
        return 0;
    }
    if (FT_New_Face(*library, font, 0, face)) {
        fprintf(stderr, "[FreeType] Error loading font face \"%s\".\n", font);
        return 0;
    }

    if (FT_Set_Pixel_Sizes(*face, 0, 16)) {
        fprintf(stderr, "[FreeType] Error setting pixel sizes.\n");
        return 0;
    }

    return 1;
}

void freetype_render_text(Atlas *atlas, Renderer *renderer, const char *text,
                          size_t text_size, Vec2f position, Vec4f color)
{
    for (size_t i = 0; i < text_size; i++) {
        int glyph_index = (unsigned char)text[i];
        if (glyph_index >= 128) {
            // unsupported non-ASCII
            glyph_index = '?';
        }

        CharacterInfo chi = atlas->metrics[glyph_index];

        float x = position.x + chi.bl;
        float y = -position.y - chi.bt;
        float w = chi.bw;
        float h = chi.bh;

        position.x += chi.ax;
        position.y += chi.ay;

        renderer_image_rectangle(
            renderer, vec2f(x, -y), vec2f(w, -h), vec2f(chi.tx, 0.0f),
            vec2f(chi.bw / atlas->width, chi.bh / atlas->height), color);
    }
}
