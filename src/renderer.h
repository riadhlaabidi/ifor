#ifndef IFOR_RENDERER_H
#define IFOR_RENDERER_H

#include <GLES3/gl3.h>
#include <stdlib.h>

#include "la.h"

#define vertex_shader_file_path "./shaders/vertex.glsl"

typedef enum {
    COLOR_SHADER = 0,
    TEXT_SHADER,
    SHADERS_COUNT,
} Shader;

typedef struct {
    Vec2f coord;
    Vec4f color;
    Vec2f texture_coord;
} Vertex;

typedef struct {
    GLuint programs[SHADERS_COUNT];
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
