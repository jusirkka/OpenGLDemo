#version 450 core

const vec4 abovewaterColor = vec4(0.25, 1.0, 1.25, 1);
const vec4 underwaterColor = vec4(0.4, 0.9, 1.0, 1);

const float ETA_A2W = 0.752;


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

struct WallData {
    vec4 c;
    vec3 n;
};

struct Cube {
    vec3 cmin;
    vec3 cmax;
};


vec2 intersectCube(const in vec3 p, const in vec3 r, const in Cube c);
WallData wallData(const in vec3 p, const in Cube c, const in sampler2D tiles);
vec4 wallColor(const in vec4 base, const in vec3 p, const in vec3 n,
               const in vec3 lp, const in Cube c,
               const in sampler2D water, const in sampler2D caustics);


vec4 surfaceColor(const in vec3 p, const in vec3 ray, const in vec4 wcol) {
    Cube c = Cube(u.cmin, u.cmax);
    vec2 t = intersectCube(p, ray, c);
    vec3 w = p + ray * t.y;
    WallData d = wallData(w, c, tiles);
    if (ray.z < 0) {
        return wcol * wallColor(d.c, w, d.n, u.lightPos, c, water, caustics);
    } else if (w.z < u.cmax.z * 0.9) {
        return wallColor(d.c, w, d.n, u.lightPos, c, water, caustics);
    }

    // vec3 color = texture(sky, ray).xyz;
    vec3 color = vec3(1.3);
    vec3 L = normalize(u.lightPos - p);
    return color + vec3(pow(max(0, dot(L, ray)), 5000.0)) * vec3(10.0, 8.0, 6.0);
}

in VS_OUT {
  vec3 pos;
  smooth vec2 surfcoord;
} fs_in;

out vec4 color;


void main(void) {

  vec2 coord = fs_in.surfcoord;
  vec4 params = texture(water, coord);
  vec3 normal = normalize(vec3(-params.z, -params.w, 1));

  for (int i = 0; i < 5; i++) {
    coord += normal.xy * 0.005;
    params = texture(water, coord);
  }


  vec3 incomingRay = normalize(fs_in.pos - u.eye);
  if (u.viewPoint > 0) {
    vec3 reflectedRay = reflect(incomingRay, normal);
    vec3 refractedRay = refract(incomingRay, normal, ETA_A2W);
    float fresnel = mix(0.25, 1.0, pow(1.0 - dot(normal, -incomingRay), 3.0));

    vec4 reflectedColor = surfaceColor(fs_in.pos, reflectedRay, abovewaterColor);
    vec4 refractedColor = surfaceColor(fs_in.pos, refractedRay, abovewaterColor);
    color = mix(refractedColor, reflectedColor, fresnel);
  } else {
    vec3 reflectedRay = reflect(incomingRay, -normal);
    vec3 refractedRay = refract(incomingRay, -normal, 1 / ETA_A2W);
    float fresnel = mix(0.5, 1.0, pow(1.0 - dot(normal, incomingRay), 3.0));

    vec4 reflectedColor = surfaceColor(fs_in.pos, reflectedRay, underwaterColor);
    vec4 refractedColor = surfaceColor(fs_in.pos, refractedRay, vec4(1)) * vec4(0.8, 1.0, 1.1, 1);

    color = mix(reflectedColor, refractedColor, (1.0 - fresnel) * length(refractedRay));
  }
}

