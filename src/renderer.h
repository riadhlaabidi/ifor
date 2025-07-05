#ifndef IFOR_RENDERER_H
#define IFOR_RENDERER_H

#include <GLES3/gl3.h>
#include <stdint.h>
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
    Vertex vertices[100000];
    size_t vertices_count;
} Renderer;

typedef enum {
    VERTEX_ATTRIBUTE_COORD2D = 0,
    VERTEX_ATTRIBUTE_COLOR,
    VERTEX_ATTRIBUTE_TEXTURE_COORD,
} VertexAttribute;

int renderer_init(Renderer *renderer, float w_width, float w_height);

void renderer_image_rectangle(Renderer *renderer, Vec2f position, Vec2f size,
                              Vec2f texture_position, Vec2f texture_size,
                              Vec4f color);

void renderer_quad(Renderer *renderer, Vec2f position1, Vec2f position2,
                   Vec2f position3, Vec2f position4, Vec4f color1, Vec4f color2,
                   Vec4f color3, Vec4f color4, Vec2f texture_position1,
                   Vec2f texture_position2, Vec2f texture_position3,
                   Vec2f texture_position4);

void renderer_triangle(Renderer *renderer, Vec2f position0, Vec2f position1,
                       Vec2f position2, Vec2f texture_position0,
                       Vec2f texture_position1, Vec2f texture_position2,
                       Vec4f color0, Vec4f color1, Vec4f color2);

void renderer_vertex(Renderer *renderer, Vec2f position, Vec2f texture_position,
                     Vec4f color);

void renderer_cleanup(Renderer *renderer);

#endif /* end of include guard: IFOR_RENDERER_H */
