#version 130

in vec4 vertex;
in vec3 normal;
in vec2 tex;

uniform mat4 n_matrix;
uniform mat4 vm_matrix;
uniform mat4 pvm_matrix;
uniform vec4 light;
uniform float shininess;
uniform mat4 bpvm_matrix;

out vec4 cosines;
out vec2 texcoord;
out vec4 shadowcoord;

void main(void) {

    texcoord = tex;
    shadowcoord = bpvm_matrix * vertex;

    vec3 N = normalize((n_matrix * vec4(normal, 0)).xyz);
    vec4 tmp =  vm_matrix * vertex;
    vec3 vertex_vm = tmp.xyz / tmp.w;
    vec3 L = normalize(light.xyz - vertex_vm);

    float d = max(0.0, dot(L,N));
    float s = 0;

    if (d > 0) {
        vec3 V = normalize(- vertex_vm);
        vec3 H = normalize(L + V);
        // Blinn-Phong
        s = pow(max(0.0, dot(H, N)), shininess);
        // Phong
        // s = pow(max(0.0, dot(V, reflect(-L, N))), shininess)
    }

    cosines = vec4(1, d, s, 0);
    gl_Position = pvm_matrix * vertex;
}
