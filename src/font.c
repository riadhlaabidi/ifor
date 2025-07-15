#include <GLES3/gl3.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "font.h"

#include "la.h"
#include "renderer.h"

#define FONT_SCALE 2
#define GLYPH_TEXTURE_PADDING 2

int freetype_init(FT_Library *library, FT_Face *face)
{
    if (FT_Init_FreeType(library)) {
        fprintf(stderr, "[FreeType] Error during library initialization.\n");
        return 0;
    }
    if (FT_New_Face(*library, font_path, 0, face)) {
        fprintf(stderr, "[FreeType] Error loading font face \"%s\".\n",
                font_path);
        return 0;
    }

    FT_UInt upscaled = font_size * FONT_SCALE;
    if (FT_Set_Pixel_Sizes(*face, 0, upscaled)) {
        fprintf(stderr, "[FreeType] Error setting pixel sizes.\n");
        return 0;
    }

    return 1;
}

int freetype_create_texture_atlas(Atlas *atlas, FT_Face face)
{
    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "[FreeType] Error loading character \'%c\'\n", i);
            return 0;
        }

        // Padding left and right for each glyph
        atlas->width += face->glyph->bitmap.width + 2 * GLYPH_TEXTURE_PADDING;
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, (GLsizei)atlas->width,
                 (GLsizei)atlas->height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, NULL);

    int x = GLYPH_TEXTURE_PADDING;
    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "[FreeType] Error loading character \'%c\'\n", i);
            return 0;
        }

        FT_GlyphSlot g = face->glyph;

        atlas->metrics[i].ax = (float)(g->advance.x >> 6) / FONT_SCALE;
        atlas->metrics[i].ay = (float)(g->advance.y >> 6) / FONT_SCALE;
        atlas->metrics[i].bw = (float)g->bitmap.width / FONT_SCALE;
        atlas->metrics[i].bh = (float)g->bitmap.rows / FONT_SCALE;
        atlas->metrics[i].bl = (float)g->bitmap_left / FONT_SCALE;
        atlas->metrics[i].bt = (float)g->bitmap_top / FONT_SCALE;
        atlas->metrics[i].tx = (float)x / (float)atlas->width;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, (GLsizei)g->bitmap.width,
                        (GLsizei)g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE,
                        g->bitmap.buffer);

        x += face->glyph->bitmap.width + 2 * GLYPH_TEXTURE_PADDING;
    }

    atlas->width /= FONT_SCALE;
    atlas->height /= FONT_SCALE;

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
            vec2f(chi.bw / (float)atlas->width, chi.bh / (float)atlas->height),
            color);

        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        renderer->vertices_count * sizeof(Vertex),
                        renderer->vertices);
        glDrawArrays(GL_TRIANGLES, 0, renderer->vertices_count);
        renderer->vertices_count = 0;
    }
}

void freetype_cleanup(FT_Library library, FT_Face face)
{
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}
