uniform  mat4 material_colors;
uniform  mat4 light_colors;
uniform float scale;

uniform sampler2D img;

varying vec2 texcoord;
varying vec4 cosines;

void main(void) {
    mat4 M = matrixCompMult(light_colors, material_colors);
    gl_FragColor = texture2D(img, texcoord * scale) +  M * cosines;
}
