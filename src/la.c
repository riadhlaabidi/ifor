#include "la.h"

Vec2f vec2f(float x, float y) { return (Vec2f){.x = x, .y = y}; }

Vec4f vec4f(float x, float y, float z, float w)
{
    return (Vec4f){.x = x, .y = y, .z = z, .w = w};
}

Vec4f vec4fu(float x) { return vec4f(x, x, x, x); }

Vec2f vec2f_add(Vec2f v1, Vec2f v2) { return vec2f(v1.x + v2.x, v1.y + v2.y); }
