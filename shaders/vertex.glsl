#version 320 es

layout(location = 0) in vec2 coord2d;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texture_coord;

out vec4 out_color;
out vec2 out_texture_coord;

void main(void) {
    gl_Position = vec4(coord2d, 0.0, 1.0);
    out_color = color;
    out_texture_coord = texture_coord;
}
