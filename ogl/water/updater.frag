#version 450 core

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

layout (binding = 0) uniform sampler2D water;

smooth in vec2 surfcoord;
// water displacement, displacement velocity, displacement gradient
layout (location = 0) out vec4 params;

void main() {

  float dq = u.updParams.x;
  float dt = u.updParams.y;
  float s = u.updParams.z * dt / dq;
  float s2 = s * s;

  /* get vertex info */
  vec4 p = texture(water, surfcoord);

  vec2 dx = vec2(dq, 0);
  vec2 dy = vec2(0, dq);

  // space and velocity gradients
  vec2 dx1 = texture(water, surfcoord - dx).xy;
  vec2 dx2 = texture(water, surfcoord + dx).xy;
  vec2 dy1 = texture(water, surfcoord - dy).xy;
  vec2 dy2 = texture(water, surfcoord + dy).xy;

  // Strictly speaking we are computing gradient of the previous frame -
  // assuming vanishingly small effect
  p.z = 0.5 * (dx2.x - dx1.x) / dq;
  p.w = 0.5 * (dy2.x - dy1.x) / dq;

  // solution of the 2D discrete wave equation

  vec2 dav = .25 * (dx1 + dx2 + dy1 + dy2) - p.xy;

  p.y += 2 * dav.x;
  p.y *= 0.995;
  p.x += p.y;
  // p.x += p.y + 2 * s2 * dav.x;
  // p.y += 2 * s2 * (2 * dav.x + dav.y);

  // p.xy *= 0.9995;

  params = p;
}
