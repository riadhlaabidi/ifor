#include <GLES3/gl3.h>
#include <stdio.h>

#include "config.h"
#include "font.h"

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

    GLuint texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // glUniform1f(uniform_texture, 0); // TODO: move that to render_text

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    return 1;
}

// static void draw_bitmap(uint32_t *data, int width, FT_Bitmap *bitmap, int x,
//                         int y)
// {
//     for (unsigned int r = 0; r < bitmap->rows; r++) {
//         for (unsigned int c = 0; c < bitmap->width; c++) {
//             int px = x + c;
//             int py = y + r;
//
//             if (px < 0 || py < 0 || px >= width)
//                 continue;
//
//             uint8_t value = bitmap->buffer[r * bitmap->pitch + c];
//             if (value == 0)
//                 continue;
//             uint32_t alpha = value;
//             uint32_t color = (alpha << 24) | 0xFFFFFF;
//             data[py * width + px] = color;
//         }
//     }
// }

// void freetype_render(uint32_t *data, int width, const char *string, int
// length)
// {
//     int penx = 0;
//     int peny = 100;
//     for (int i = 0; i < length; i++) {
//         if (FT_Load_Char(face, string[i], FT_LOAD_RENDER)) {
//             fprintf(stderr, "[FreeType] Error loading char glyph '%c'\n",
//                     string[i]);
//             continue;
//         }
//
//         // draw
//         draw_bitmap(data, width, &face->glyph->bitmap,
//                     penx + face->glyph->bitmap_left,
//                     peny - face->glyph->bitmap_top);
//
//         penx += face->glyph->advance.x >> 6;
//     }
// }
//
// void freetype_cleanup(void) {}
