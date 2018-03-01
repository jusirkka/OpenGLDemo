#version 450 core

const float ETA_A2W = 0.752;

struct Cube {
    vec3 cmin;
    vec3 cmax;
};

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


vec2 intersectCube(const in vec3 p, const in vec3 r, const in Cube c);

in VS_OUT {
  vec3 opos;
  smooth vec3 npos;
  vec3 incidentLight;
  vec3 pos;
} fs_in;

layout (location = 0) out vec4 color;

void main() {
  /* if the triangle gets smaller, it gets brighter, and vice versa */
  float oldArea = length(dFdx(fs_in.opos)) * length(dFdy(fs_in.opos));
  float newArea = length(dFdx(fs_in.npos)) * length(dFdy(fs_in.npos));
  color = vec4(oldArea / newArea * .2, 1, 0, 0);
  // color = vec4(.2 , 1, 0, 0);
  vec3 refractedLight = refract(fs_in.incidentLight, vec3(0, 0, 1), ETA_A2W);

  /* shadow for the rim of the pool */
  vec2 t = intersectCube(fs_in.npos, refractedLight, Cube(u.cmin, u.cmax));
  // zero for rays that first intersect the box at z = cmax.z
  float p0 = (fs_in.npos.z + refractedLight.z * t.x) - u.cmax.z;
  color.r *= 2 / (1 + exp(-200 / (1 + 10 * (t.y - t.x)) * p0));
  // color = vec4(1);
}
