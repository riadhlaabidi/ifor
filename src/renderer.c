#include <GLES3/gl3.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "renderer.h"

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

int renderer_init(Renderer *renderer)
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

    // renderer->vertices[renderer->vertices_count++] =
    //     (Vertex){.coord = vec2f(-1, 1), .color = vec4fu(1.0)};
    // renderer->vertices[renderer->vertices_count++] =
    //     (Vertex){.coord = vec2f(-1, 0), .color = vec4fu(1.0)};
    // renderer->vertices[renderer->vertices_count++] =
    //     (Vertex){.coord = vec2f(0, 0), .color = vec4fu(1.0)};

    GLuint vs = create_shader(vertex_shader_path, GL_VERTEX_SHADER);
    GLuint fs = create_shader(fragment_shader_path, GL_FRAGMENT_SHADER);
    GLuint text_fs = create_shader(text_fragment_shader_path,
                                   GL_FRAGMENT_SHADER);

    if (!vs || !fs || !text_fs) {
        return 0;
    }

    GLint link = GL_FALSE;
    renderer->programs[0] = glCreateProgram();
    glAttachShader(renderer->programs[0], vs);
    glAttachShader(renderer->programs[0], fs);
    glLinkProgram(renderer->programs[0]);
    glGetProgramiv(renderer->programs[0], GL_LINK_STATUS, &link);

    if (!link) {
        fprintf(stderr, "Error in program linking.\n");
        log_program_info(renderer->programs[0]);
        return 0;
    }

    renderer->programs[1] = glCreateProgram();
    glAttachShader(renderer->programs[1], vs);
    glAttachShader(renderer->programs[1], text_fs);
    glLinkProgram(renderer->programs[1]);
    glGetProgramiv(renderer->programs[1], GL_LINK_STATUS, &link);

    if (!link) {
        fprintf(stderr, "Error in program linking.\n");
        log_program_info(renderer->programs[1]);
        return 0;
    }

    return 1;
}

void render(Renderer *renderer)
{
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderer->programs[0]);
    glLineWidth(1);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    renderer->vertices_count * sizeof(Vertex),
                    renderer->vertices);
    glDrawArrays(GL_TRIANGLES, 0, renderer->vertices_count);
    renderer->vertices_count = 0;
}

void renderer_cleanup(Renderer *renderer)
{
    glDisableVertexAttribArray(VERTEX_ATTRIBUTE_COORD2D);
    glDisableVertexAttribArray(VERTEX_ATTRIBUTE_COLOR);
    glDisableVertexAttribArray(VERTEX_ATTRIBUTE_TEXTURE_COORD);

    glDeleteProgram(renderer->programs[0]);
    glDeleteProgram(renderer->programs[1]);
    glDeleteBuffers(1, &renderer->vbo);
}
