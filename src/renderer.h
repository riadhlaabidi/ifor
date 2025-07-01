#ifndef IFOR_RENDERER_H
#define IFOR_RENDERER_H

#include <GLES3/gl3.h>
#include <stddef.h>

#include "la.h"

#define vertex_shader_path "./shaders/vertex.glsl"
#define fragment_shader_path "./shaders/fragment.glsl"
#define text_fragment_shader_path "./shaders/text-fragment.glsl"

typedef struct {
    Vec2f coord;
    Vec4f color;
    Vec2f texture_coord;
} Vertex;

typedef struct {
    GLuint programs[2];
    GLuint vbo;

    GLint uniforms[1];
    Vertex vertices[1000];
    size_t vertices_count;
} Renderer;

typedef enum {
    VERTEX_ATTRIBUTE_COORD2D = 0,
    VERTEX_ATTRIBUTE_COLOR,
    VERTEX_ATTRIBUTE_TEXTURE_COORD,
} VertexAttribute;

int renderer_init(Renderer *renderer);
void render(Renderer *renderer);
void renderer_cleanup(Renderer *renderer);

#endif /* end of include guard: IFOR_RENDERER_H */
