#include <GLES3/gl3.h>
#include <stddef.h>
#include <stdio.h>

#include "common.h"
#include "la.h"
#include "renderer.h"

const char *shader_file_paths[SHADERS_COUNT] = {
    [COLOR_SHADER] = "./shaders/fragment.glsl",
    [TEXT_SHADER] = "./shaders/text-fragment.glsl",
};

static void log_program_info(GLuint prg)
{
    GLsizei msg_length = 0;
    glGetProgramiv(prg, GL_INFO_LOG_LENGTH, &msg_length);
    if (msg_length > 0) {
        char *message = (char *)malloc(msg_length);
        glGetProgramInfoLog(prg, msg_length, NULL, message);
        fprintf(stderr, "%s\n", message);
        free(message);
    }
}

static void log_shader_info(GLuint shader)
{
    GLsizei msg_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &msg_length);
    if (msg_length > 0) {
        char *message = (char *)malloc(msg_length);
        glGetShaderInfoLog(shader, msg_length, NULL, message);
        fprintf(stderr, "%s\n", message);
        free(message);
    }
}

static GLuint create_shader(const char *path, GLenum type)
{
    GLuint shader = 0;

    const GLchar *src = read_file(path);
    if (!src) {
        fprintf(stderr, "Error reading file \"%s\"\n", path);
        goto defer;
    }

    shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint compile = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile);
    if (!compile) {
        fprintf(stderr, "Error compiling shader file \"%s\"\n", path);
        log_shader_info(shader);
        glDeleteShader(shader);
    }

defer:
    free((void *)src);
    return shader;
}

int renderer_init(Renderer *renderer, float w_width, float w_height)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenBuffers(1, &renderer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->vertices),
                 renderer->vertices, GL_DYNAMIC_DRAW);

    // coord2d
    glEnableVertexAttribArray(VERTEX_ATTRIBUTE_COORD2D);
    glVertexAttribPointer(VERTEX_ATTRIBUTE_COORD2D, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (GLvoid *)offsetof(Vertex, coord));

    // color
    glEnableVertexAttribArray(VERTEX_ATTRIBUTE_COLOR);
    glVertexAttribPointer(VERTEX_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (GLvoid *)offsetof(Vertex, color));

    // texture coord
    glEnableVertexAttribArray(VERTEX_ATTRIBUTE_TEXTURE_COORD);
    glVertexAttribPointer(VERTEX_ATTRIBUTE_TEXTURE_COORD, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex),
                          (GLvoid *)offsetof(Vertex, texture_coord));

    GLuint vs = create_shader(vertex_shader_file_path, GL_VERTEX_SHADER);
    if (!vs) {
        return 0;
    }

    float u_projection[16];
    orthographic_matrix(0.0f, (float)w_width, 0.0f, (float)w_height, -1, 1,
                        u_projection);

    for (int i = 0; i < SHADERS_COUNT; i++) {
        GLuint fs = create_shader(shader_file_paths[i], GL_FRAGMENT_SHADER);

        if (!fs) {
            return 0;
        }

        GLint link = GL_FALSE;
        renderer->programs[i] = glCreateProgram();
        glAttachShader(renderer->programs[i], vs);
        glAttachShader(renderer->programs[i], fs);
        glLinkProgram(renderer->programs[i]);
        glGetProgramiv(renderer->programs[i], GL_LINK_STATUS, &link);

        if (!link) {
            fprintf(stderr, "Error in program linking.\n");
            log_program_info(renderer->programs[i]);
            return 0;
        }

        glUseProgram(renderer->programs[i]);

        GLchar *u_projection_name = "projection";
        GLint u_projection_loc = glGetUniformLocation(renderer->programs[i],
                                                      u_projection_name);
        if (u_projection_loc == -1) {
            fprintf(stderr,
                    "Error getting location of uniform \"%s\" in program %d\n",
                    u_projection_name, i);
            return 0;
        }

        glUniformMatrix4fv(u_projection_loc, 1, GL_FALSE, u_projection);

        glDeleteShader(fs);
    }

    glDeleteShader(vs);

    return 1;
}

void renderer_vertex(Renderer *renderer, Vec2f position, Vec2f texture_position,
                     Vec4f color)
{
    Vertex *last = &renderer->vertices[renderer->vertices_count];
    last->coord = position;
    last->texture_coord = texture_position;
    last->color = color;
    renderer->vertices_count++;
}

void renderer_triangle(Renderer *renderer, Vec2f position0, Vec2f position1,
                       Vec2f position2, Vec2f texture_position0,
                       Vec2f texture_position1, Vec2f texture_position2,
                       Vec4f color0, Vec4f color1, Vec4f color2)
{
    renderer_vertex(renderer, position0, texture_position0, color0);
    renderer_vertex(renderer, position1, texture_position1, color1);
    renderer_vertex(renderer, position2, texture_position2, color2);
}

void renderer_quad(Renderer *renderer, Vec2f position0, Vec2f position1,
                   Vec2f position2, Vec2f position3, Vec4f color0, Vec4f color1,
                   Vec4f color2, Vec4f color3, Vec2f texture_position0,
                   Vec2f texture_position1, Vec2f texture_position2,
                   Vec2f texture_position3)
{
    renderer_triangle(renderer, position0, position1, position2,
                      texture_position0, texture_position1, texture_position2,
                      color0, color1, color2);
    renderer_triangle(renderer, position1, position2, position3,
                      texture_position1, texture_position2, texture_position3,
                      color1, color2, color3);
}

void renderer_image_rectangle(Renderer *renderer, Vec2f position, Vec2f size,
                              Vec2f texture_position, Vec2f texture_size,
                              Vec4f color)
{
    renderer_quad(renderer, position, vec2f_add(position, vec2f(size.x, 0)),
                  vec2f_add(position, vec2f(0, size.y)),
                  vec2f_add(position, size), color, color, color, color,
                  texture_position,
                  vec2f_add(texture_position, vec2f(texture_size.x, 0)),
                  vec2f_add(texture_position, vec2f(0, texture_size.y)),
                  vec2f_add(texture_position, texture_size));
}

void renderer_solid_rectanlge(Renderer *renderer, Vec2f position, Vec2f size,
                              Vec4f color)
{
    Vec2f empty = vec2f(0, 0);
    renderer_quad(renderer, position, vec2f_add(position, vec2f(size.x, 0)),
                  vec2f_add(position, vec2f(0, size.y)),
                  vec2f_add(position, size), color, color, color, color, empty,
                  empty, empty, empty);
}

void renderer_cleanup(Renderer *renderer)
{
    glDisableVertexAttribArray(VERTEX_ATTRIBUTE_COORD2D);
    glDisableVertexAttribArray(VERTEX_ATTRIBUTE_COLOR);
    glDisableVertexAttribArray(VERTEX_ATTRIBUTE_TEXTURE_COORD);

    for (int i = 0; i < SHADERS_COUNT; i++) {
        glDeleteProgram(renderer->programs[i]);
    }

    glDeleteBuffers(1, &renderer->vbo);
}
