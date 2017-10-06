#version 130

in vec4 vertex;

uniform mat4 pvm_matrix;

void main(void) {
    gl_Position = pvm_matrix * vertex;
}
