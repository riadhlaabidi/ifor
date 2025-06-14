#ifndef IFOR_RENDERER_H
#define IFOR_RENDERER_H

#include "ifor.h"

#define vertex_shader_path "./shaders/vertex.glsl"
#define fragment_shader_path "./shaders/fragment.glsl"

int gl_init(void);
void render(IFOR_state *state);
void gl_cleanup(void);

#endif /* end of include guard: IFOR_RENDERER_H */
