#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "renderer.h"

GLuint program;
GLuint vbo;
GLint attribute_coord2d;

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

int gl_init(void)
{
    /* Enable alpha */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* VBO */
    GLfloat triangle_vertices[] = {0.0, 0.8, -0.8, -0.8, 0.8, -0.8};
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices,
                 GL_DYNAMIC_DRAW);

    GLint link = GL_FALSE;

    GLuint vs = create_shader(vertex_shader_path, GL_VERTEX_SHADER);
    GLuint fs = create_shader(fragment_shader_path, GL_FRAGMENT_SHADER);
    if (!vs || !fs) {
        return 0;
    }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link);

    if (!link) {
        fprintf(stderr, "Error in program linking.\n");
        log_program_info(program);
        return 0;
    }

    const char *attribute_name = "coord2d";
    attribute_coord2d = glGetAttribLocation(program, attribute_name);

    if (attribute_coord2d == -1) {
        fprintf(stderr, "Could not bind attribute \"%s\" in program\n",
                attribute_name);
        return 0;
    }

    return 1;
}

void render(IFOR_state *state)
{
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(attribute_coord2d);
    glVertexAttribPointer(attribute_coord2d, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(attribute_coord2d);
    eglSwapBuffers(state->egl_display, state->egl_surface);
}

void gl_cleanup(void)
{
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
}
