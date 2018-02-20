#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
    vec3 normal;
    vec4 color;
    int index;
} gs_in[];

out GS_OUT
{
    vec3 normal;
    vec4 color;
} gs_out;

struct explode_t {
    mat4 rot;
    vec4 tr;
};

layout (std140) uniform explosions
{
    explode_t t[200];
};

uniform int xplode = 1;

void main(void) {
    if (xplode == 1) {
        vec4 p0 = gl_in[0].gl_Position;
        vec3 x = normalize(gl_in[1].gl_Position.xyz - p0.xyz);
        vec3 y0 = gl_in[2].gl_Position.xyz - p0.xyz;
        vec3 y = normalize(y0 - x * dot(x, y0));
        vec3 z = cross(x, y);
        mat4 m = mat4(x, 0, y, 0, z, 0, vec4(0, 0, 0, 1));

        for (int i = 0; i < gl_in.length(); i++) {
            int j = gs_in[i].index % 200;
            gl_Position = p0 + m * t[j].rot * (gl_in[i].gl_Position - p0) * m + m * t[j].tr;
            gs_out.normal = gs_in[i].normal;
            gs_out.color = gs_in[i].color;
            EmitVertex();
        }
    } else {
        for (int i = 0; i < gl_in.length(); i++) {
            gl_Position = gl_in[i].gl_Position;
            gs_out.normal = gs_in[i].normal;
            gs_out.color = gs_in[i].color;
            EmitVertex();
        }
    }
    EndPrimitive();
}
