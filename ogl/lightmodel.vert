attribute vec4 vertex;
attribute vec3 normal;
attribute vec2 tex;

uniform  mat4 n_matrix;
uniform  mat4 vm_matrix;
uniform  mat4 pvm_matrix;
uniform  vec4 light;
uniform float specular;

varying vec4 cosines;
varying vec2 texcoord;

void main(void) {

    texcoord = tex;

    vec4 N = normalize(n_matrix * vec4(normal, 0));
    vec4 V = vec4(normalize((vm_matrix * vertex).xyz), 1);
    vec4 L = normalize(light);

    cosines = vec4(1, max(0.0, dot(N,L)), pow(max(0.0, dot(V, reflect(L, N))), specular), 0);
    gl_Position = pvm_matrix * vertex;
}
