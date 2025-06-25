#version 100

#ifdef GL_FRAGMENT_PRECISION_HIGH
    precision highp float;
#else
    precision mediump float;
#endif

varying vec3 f_color;
uniform float fade;

void main(void) {
  gl_FragColor = vec4(f_color.r, f_color.g, f_color.b, fade);
};
