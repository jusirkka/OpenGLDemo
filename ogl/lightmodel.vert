attribute vec4 vertex;
attribute vec3 normal;
attribute vec2 tex;

uniform  mat4 n_matrix;
uniform  mat4 vm_matrix;
uniform  mat4 pvm_matrix;
uniform  vec4 light;
uniform float shininess;

varying vec4 cosines;
varying vec2 texcoord;

void main(void) {

    texcoord = tex;

    vec3 N = normalize((n_matrix * vec4(normal, 0)).xyz);
    vec3 V = normalize((vm_matrix * vertex).xyz);
    vec3 L = normalize(light.xyz); // infinite distance

    cosines = vec4(1, max(0.0, dot(L,N)), pow(max(0.0, dot(V, reflect(L, N))), shininess), 0);
    gl_Position = pvm_matrix * vertex;
}
