#include "la.h"

Vec2f vec2f(float x, float y) { return (Vec2f){.x = x, .y = y}; }

Vec4f vec4f(float x, float y, float z, float w)
{
    return (Vec4f){.x = x, .y = y, .z = z, .w = w};
}

Vec4f vec4fu(float x) { return vec4f(x, x, x, x); }

Vec2f vec2f_add(Vec2f v1, Vec2f v2) { return vec2f(v1.x + v2.x, v1.y + v2.y); }

void orthographic_matrix(float left, float right, float bottom, float top,
                         float near, float far, float *matrix)
{
    matrix[0] = 2.0f / (right - left); // scale x
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 2.0f / (top - bottom); // scale y
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -2.0f / (far - near); // scale z
    matrix[11] = 0.0f;

    matrix[12] = -(right + left) / (right - left); // translate x
    matrix[13] = -(top + bottom) / (top - bottom); // translate y
    matrix[14] = -(far + near) / (far - near);     // translate z
    matrix[15] = 1.0f;
}
