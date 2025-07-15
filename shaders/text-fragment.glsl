#version 320 es
    
#ifdef GL_FRAGMENT_PRECISION_HIGH
    precision highp float;
#else
    precision mediump float;
#endif

uniform sampler2D texture_image;

in vec2 out_texture_coord;
in vec4 out_color;

out vec4 frag_color;

void main(void) {
    frag_color = vec4(1, 1, 1, texture(texture_image, out_texture_coord).a) * out_color; 
}
