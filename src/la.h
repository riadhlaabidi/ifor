#ifndef IFOR_LA_H
#define IFOR_LA_H

typedef struct {
    float x;
    float y;
} Vec2f;

Vec2f vec2f(float x, float y);
Vec2f vec2f_add(Vec2f v1, Vec2f v2);

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec4f;

Vec4f vec4f(float x, float y, float z, float w);
Vec4f vec4fu(float x);

#endif /* end of include guard: IFOR_LA_H */
