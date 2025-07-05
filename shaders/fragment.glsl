#version 320 es

#ifdef GL_FRAGMENT_PRECISION_HIGH
    precision highp float;
#else
    precision mediump float;
#endif

in vec4 out_color;
out vec4 frag_color;

void main(void) {
  frag_color = out_color;
}
