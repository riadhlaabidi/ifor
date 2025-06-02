#ifndef IFOR_FONT_H
#define IFOR_FONT_H

#include <stdint.h>

int freetype_init(void);
void render(uint32_t *data, int width, const char *string, int length);
void freetype_cleanup(void);

#endif /* end of include guard: IFOR_FONT_H */
