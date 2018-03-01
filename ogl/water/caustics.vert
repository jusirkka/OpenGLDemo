#version 450 core

const float ETA_A2W = .752;
const float EPS = .0001;

layout (binding = 0) uniform sampler2D water;
layout (location = 0) in vec4 vertex;
layout (location = 2) in vec2 tex;

out VS_OUT {
  vec3 opos;
  smooth vec3 npos;
  vec3 incidentLight;
  vec3 pos;
} vs_out;

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


vec2 intersectHalfCube(const in vec3 p, const in vec3 r, float h) {
    vec3 cmax = vec3(u.cmax.xy, h);
    vec3 tMin = (u.cmin - p) / r;
    vec3 tMax = (cmax - p) / r;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}


/* project the ray onto the floor */
vec3 project(const in vec3 p, const in vec3 ray1, const in vec3 ray2, float h) {
  // small h > p.z ensures that the points p.z ~ 0 have front intersection at p.z = h
  // and the 'real' back intersection
  vec2 t = intersectHalfCube(p, ray1, h);
  // back intersection
  vec3 w = p + t.y * ray1;
  float tb = (u.cmin.z - w.z) / ray2.z;
  return w + tb * ray2;
}


void main() {
  vec4 params = texture(water, tex);
  vec3 normal = normalize(vec3(-params.z, -params.w, 1));
  vs_out.pos = (u.model * vertex).xyz;
  vs_out.incidentLight = normalize(vs_out.pos - u.lightPos);


  /* project the vertices along the refracted vertex ray */
  vec3 refr_calm = refract(vs_out.incidentLight, vec3(0, 0, 1), ETA_A2W);
  vec3 refr = refract(vs_out.incidentLight, normal, ETA_A2W);
  vs_out.opos = project(vs_out.pos, refr_calm, refr_calm, EPS);
  vs_out.npos = project(vs_out.pos + vec3(0, 0, params.x), refr, refr_calm, abs(params.x) + EPS);


  float wl = - u.cmin.z / refr_calm.z;
  // pos is mapped to c, not necessarily inside the source square.
  // Assume that target square is abs(c.xy) < 3
  vec2 c = vs_out.npos.xy + wl * refr_calm.xy / refr_calm.z;
  // device independent coords
  gl_Position = vec4(0.333 * c.xy, 0, 1);
}
