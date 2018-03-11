#version 450 core

layout (lines) in;
layout (triangle_strip, max_vertices = 256) out;

in TE_OUT {
    vec3 y0;
    vec3 x0;
    float t;
} gs_in[];


out GS_OUT {
    vec2 tex;
    float diffuse;
} gs_out;


const int NUM_V = 20;
const float R = .2;

uniform mat4 m_p;
uniform mat4 m_v;
uniform vec4 lp;

void main(void) {

    vec3 x0[2];
    vec3 y0[2];

   y0[0] = gs_in[0].y0;
   x0[0] = gs_in[0].x0;
   if (dot(gs_in[0].y0, gs_in[1].y0) < 0) {
       y0[1] = - gs_in[1].y0;
       x0[1] = - gs_in[1].x0;
   } else {
       y0[1] = gs_in[1].y0;
       x0[1] = gs_in[1].x0;
   }

    for (int k = 0; k < NUM_V; k++) {
       float s = float(k) / (NUM_V - 1);
       for (int i = 0; i < 2; i++) {
           float a = radians(360. * s);
           vec3 N = (cos(a) * x0[i] + sin(a) * y0[i]);
           vec3 pos = gl_in[i].gl_Position.xyz + R * N;
           vec4 v = m_v * vec4(pos, 1);
           vec3 L = normalize(lp.xyz - v.xyz);
           // assuming mat3(m_v) is pure rotation matrix
           gs_out.diffuse = max(0, dot(L, mat3(m_v) * N));
           gs_out.tex = vec2(s, gs_in[i].t);
           gl_Position = m_p * v;
           EmitVertex();
       }
    }
    EndPrimitive();
}
