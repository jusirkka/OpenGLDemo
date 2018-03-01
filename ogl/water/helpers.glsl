#version 450 core

const float ETA_A2W = 0.752;
const float EPS = 0.001;



struct Cube {
    vec3 cmin;
    vec3 cmax;
};


vec2 intersectCube(const in vec3 p, const in vec3 ray, const in Cube c) {
    vec3 tMin = (c.cmin - p) / ray;
    vec3 tMax = (c.cmax - p) / ray;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}

vec2 toTex(const in vec2 p, const in Cube c) {
    return (p - c.cmin.xy) / (c.cmax.xy - c.cmin.xy);
}


vec4 wallColor(const in vec4 base, const in vec3 p, const in vec3 n,
               const in vec3 lp, const in Cube c,
               const in sampler2D water, const in sampler2D caustics) {
    float scale = .4;


    scale /= length(p); /* pool ambient occlusion */

    vec3 incidentLight = normalize(p - lp);
    vec3 refractedLight = refract(incidentLight, vec3(0, 0, 1), ETA_A2W);

    float wlev = texture2D(water, toTex(p.xy, c)).x;
    /* caustics */
    if (p.z < wlev) {
      float diffuse = max(0.0, dot(-refractedLight, n));
      if (diffuse > 0) {
          // caustics: k
          vec2 k = p.xy - p.z * refractedLight.xy / refractedLight.z;
          vec4 caustic = texture2D(caustics, (.33 * k + 1) / 2);
          // vec4 caustic = texture2D(caustics, toTex(ETA_A2W * k, c));
          // vec4 caustic = texture2D(caustics, toTex(p.xy, c));
          // r: rim shadow, g: caustics
          diffuse *= caustic.r * caustic.g;
          scale += 2 * diffuse;
      }
    } else {
      float diffuse = max(0.0, dot(-incidentLight, n));
      if (diffuse > 0) {
          vec2 t = intersectCube(p, incidentLight, c);
          // zero for rays that first intersect the box at z = cmax.z
          float p0 = (p.z + incidentLight.z * t.x) - c.cmax.z;
          diffuse *= 1 / (1 + exp(-200 / (1 + 10 * (t.y - t.x)) * p0));
          scale += 2 * diffuse;
      }
    }

    return base * scale;
}

struct WallData {
    vec4 c;
    vec3 n;
};

WallData wallData(const in vec3 p, const in Cube c, const in sampler2D tiles) {
    vec3 t = 2 * (p - c.cmin) / (c.cmax - c.cmin) - 1;

    if (abs(t.x) > 1 - EPS) return WallData(texture(tiles, (t.yz + 1) / 2), vec3(0, -t.x, 0));
    if (abs(t.y) > 1 - EPS) return WallData(texture(tiles, (t.xz + 1) / 2), vec3(0, -t.y, 0));
    if (t.z < -1 + EPS) return WallData(texture(tiles, (t.yx + 1) / 2), vec3(0, 0, 1));
    return WallData(vec4(10), vec3(0, 0, -1));
}

