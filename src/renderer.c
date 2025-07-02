#include <GLES3/gl3.h>
#include <stddef.h>
#include <stdio.h>

#include "common.h"
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

    GLuint vs = create_shader(vertex_shader_file_path, GL_VERTEX_SHADER);
    if (!vs) {
        return 0;
    }

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

        glDeleteShader(fs);
    }

    glDeleteShader(vs);
    return 1;
}

void render(Renderer *renderer)
{
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(renderer->programs[COLOR_SHADER]);
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

    for (int i = 0; i < SHADERS_COUNT; i++) {
        glDeleteProgram(renderer->programs[i]);
    }

    glDeleteBuffers(1, &renderer->vbo);
}
