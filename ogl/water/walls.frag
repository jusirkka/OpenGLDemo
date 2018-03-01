#version 450 core

const vec4 underwaterColor = vec4(0.4, 0.9, 1.0, 1);


layout (binding = 0) uniform sampler2D water;
layout (binding = 1) uniform sampler2D tiles;
layout (binding = 2) uniform sampler2D caustics;


layout (std140) uniform UForms {
  mat4 pv;
  vec3 lightPos;
  vec3 cmin;
  vec3 cmax;
  vec3 eye;
  mat4 model;
  mat3 normal_t;
  vec4 p1;
  vec4 updParams; // spatial  diff dx, time diff dt, wave speed
  vec4 drop;
  float viewPoint;
} u;


struct Cube {
    vec3 cmin;
    vec3 cmax;
};


vec4 wallColor(const in vec4 base, const in vec3 p, const in vec3 n,
               const in vec3 lp, const in Cube c,
               const in sampler2D water, const in sampler2D caustics);

vec2 toTex(const in vec2 p, const in Cube c);

in VS_OUT {
  vec3 pos;
  vec3 normal;
  vec2 texcoord;
} fs_in;

out vec4 color;

void main() {
  vec4 base = texture(tiles, fs_in.texcoord);
  Cube c = Cube(u.cmin, u.cmax);
  color = wallColor(base, fs_in.pos, -fs_in.normal, u.lightPos, c,
                    water, caustics);
  float wlev = texture(water, toTex(fs_in.pos.xy, c)).x;
  if (fs_in.pos.z < wlev) {
    color *= underwaterColor * 1.2;
  }
}
