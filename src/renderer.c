#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "renderer.h"

GLuint program;
GLuint vbo_triangles;
GLuint vbo_colors;
GLint attribute_coord2d;
GLint attribute_v_color;
GLint uniform_fade;

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
    GLfloat triangle_vertices[] = {1, 1, -1, 1, -1, -1};
    glGenBuffers(1, &vbo_triangles);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangles);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices,
                 GL_DYNAMIC_DRAW);

    GLfloat triangle_colors[] = {1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0};
    glGenBuffers(1, &vbo_colors);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_colors), triangle_colors,
                 GL_STATIC_DRAW);

    GLuint vs = create_shader(vertex_shader_path, GL_VERTEX_SHADER);
    GLuint fs = create_shader(fragment_shader_path, GL_FRAGMENT_SHADER);
    if (!vs || !fs) {
        return 0;
    }

    GLint link = GL_FALSE;
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

    attribute_name = "v_color";
    attribute_v_color = glGetAttribLocation(program, attribute_name);

    if (attribute_v_color == -1) {
        fprintf(stderr, "Could not bind attribute \"%s\" in program\n",
                attribute_name);
        return 0;
    }

    const char *uniform_name = "fade";
    uniform_fade = glGetUniformLocation(program, uniform_name);
    if (uniform_fade == -1) {
        fprintf(stderr, "Could not bind uniform \"%s\" in program\n",
                uniform_name);
        return 0;
    }

    return 1;
}

void render(IFOR_state *state)
{
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangles);
    glEnableVertexAttribArray(attribute_coord2d);
    glVertexAttribPointer(attribute_coord2d, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attribute_v_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_colors);
    glVertexAttribPointer(attribute_v_color, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(attribute_coord2d);
    glDisableVertexAttribArray(attribute_v_color);

    glUniform1f(uniform_fade, 1.0);

    eglSwapBuffers(state->egl_display, state->egl_surface);
}

void gl_cleanup(void)
{
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_triangles);
}
