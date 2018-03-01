#version 450 core
const float PI = 3.141592653589793;

layout (binding = 0) uniform sampler2D water;

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

smooth in vec2 surfcoord;
// water displacement, displacement velocity, displacement gradient
layout (location = 0) out vec4 params;

void main() {

  vec4 p = texture(water, surfcoord);

  /* add the drop to the height */
  vec2 c = u.drop.xy;
  float r = u.drop.z;
  float h = u.drop.w;
  float dt = u.updParams.y;
  float v = u.updParams.z;
  float arg = max(0., 1. - length(c - surfcoord) / r);

  p.x += h * (1 - cos(arg * PI)) / 2;
  p.y += - v * dt * PI * h / r * sin(arg * PI) / 2;

  params = p;

}
