#version 120

uniform vec4 iResolution;
uniform float iTime;
uniform mat4 iWindowToView;
uniform mat4 iViewToWorld;
uniform float iEarthRadius;


const int MAX_MARCHING_STEPS = 255;
const float MIN_DIST = 0.0;
const float MAX_DIST = 600.0;
const float EPSILON = 0.0001;
const float NORMAL_DELTA = 0.1;
const float PI = 3.1415926535897932384626433832795;
const float MUSHROOM_RADIUS = 10;


const int NO_HITS = -1;
const int THING = 1;
const int SKY = 3;
const int EARTH = 4;

struct Ray {
    vec3 eye;
    vec3 dir;
};

struct HitInfo {
    int object;
    float dist;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 pos;
    vec3 color;
};

// ramp function with period = 2
float ramp(float x) {
    return 2. * mod(x, 1.) - 1.;
}

// square function with on/off values a & b, period 2
float square(float a, float b, float x) {
    return mix(a, b, step(ramp(x), 0.));
}

// http://www.iquilezles.org/www/articles/smin/smin.htm
float smin(float a, float b, float k) {
    float h = clamp(.5 + .5 * (b - a) / k, 0., 1.);
    return mix(b, a, h) - k * h * (1. - h);
}

float smax(float a, float b, float k) {
    float h = clamp(.5 + .5 * (b - a) / k, 0., 1.);
    return mix(a, b, h) + k * h * (1. - h);
}

/**
 * Constructive solid geometry intersection operation on SDF-calculated distances.
 */
HitInfo intersectH(const in HitInfo a, const in HitInfo b) {
    if (a.dist > b.dist) return a;
    return b;
}

/**
 * Constructive solid geometry union operation on SDF-calculated distances.
 */
HitInfo unionH(const in HitInfo a, const in HitInfo b) {
    if (a.dist < b.dist) return a;
    return b;
}


HitInfo unionSH(const in HitInfo a, const in HitInfo b) {
    return HitInfo(a.dist < b.dist ? a.object : b.object, smin(a.dist, b.dist, .5));
}

HitInfo unionS1(const in HitInfo a, const in HitInfo b) {
    return HitInfo(a.dist < b.dist ? a.object : b.object, smin(a.dist, b.dist, 1.));
}

HitInfo unionS2(const in HitInfo a, const in HitInfo b) {
    return HitInfo(a.dist < b.dist ? a.object : b.object, smin(a.dist, b.dist, 2.));
}

/**
 * Constructive solid geometry difference operation on SDF-calculated distances.
 */
HitInfo differenceH(const in HitInfo a, const in HitInfo b) {
    if (a.dist > - b.dist) return a;
    return HitInfo(b.object, -b.dist);
}

HitInfo differenceSH(const in HitInfo a, const in HitInfo b) {
    return HitInfo(a.dist > - b.dist ? a.object : b.object, smax(a.dist, -b.dist, .5));
}

HitInfo differenceS1(const in HitInfo a, const in HitInfo b) {
    return HitInfo(a.dist > - b.dist ? a.object : b.object, smax(a.dist, -b.dist, 1.));
}

HitInfo differenceS2(const in HitInfo a, const in HitInfo b) {
    return HitInfo(a.dist > - b.dist ? a.object : b.object, smax(a.dist, -b.dist, 2.));
}


float sphereSDF(const in vec3 p) {
    return length(p) - 1.0;
}


float plateSDF(const in vec2 p, const in vec2 dim) {
    vec2 d = (abs(p) - dim);
    return max(d.x, d.y);
}

float cylinderSDF(const in vec3 p, const in vec2 h) {
    return plateSDF(vec2(length(p.xy), p.z), h);
}


vec3 rotateX(const in vec3 p, float a) {
    float s = sin(a);
    float c = cos(a);
    return vec3(p.x, p.y*c - p.z*s, p.y*s + p.z*c);
}

vec3 rotateZ(const in vec3 p, float a) {
    float s = sin(a);
    float c = cos(a);
    return vec3(p.x*c - p.y*s, p.x*s + p.y*c, p.z);
}

vec3 repeatE(const in vec3 p) {
    float a = mod(atan(p.y, p.z) + PI, PI / 8) - PI / 16;
    float r = length(p.yz);
    return vec3(p.x, r*sin(a), r*cos(a));
}


vec3 moveTo(const vec3 p, vec2 pos) {
    vec3 coe = vec3(0, 0, -iEarthRadius);
    // 0: latitude, 1: longitude
    return repeatE(rotateX(rotateZ(p - coe, radians(pos[1])), radians(90 - pos[0]))) + coe;
}


HitInfo mushroom(in HitInfo ground, const in vec3 p, const in vec2 p0) {
    vec3 q;
    const float rob = MUSHROOM_RADIUS;
    const float rou = .5 * MUSHROOM_RADIUS;
    const float rol = .6 * MUSHROOM_RADIUS;
    const vec2 cdim = vec2(.5, 6.);

    q = moveTo(p, p0)  + vec3(.0, .0, .6 * rob - cdim[1]);
    HitInfo mc = HitInfo(THING, cylinderSDF(q, cdim));
    ground = unionS2(ground, mc);

    q += vec3(.0, .0, -cdim[1] + rou * .95);
    HitInfo mhu = HitInfo(THING, sphereSDF(q / rou) * rou);
    q += vec3(.0, .0, mix(1.5, 2.0, 1 + sin(2.*iTime)));
    HitInfo mhl = HitInfo(THING, sphereSDF(q / rol) * rol);

    return unionH(ground, differenceSH(mhu , mhl));

}


HitInfo ground(in HitInfo earth, const in vec3 p, const in vec2 p0) {
    vec3 q;
    const float rob = MUSHROOM_RADIUS;

    q = moveTo(p, p0)  - vec3(.0, .0, .4 * rob);
    HitInfo mb = HitInfo(EARTH, sphereSDF(q / rob) * rob);

    return differenceS2(earth, mb);
}

HitInfo sceneH(const in vec3 p) {
    vec3 q;

    vec3 coe = vec3(0., 0., -iEarthRadius);

    vec2 p1 = vec2(70, 90) + iTime * vec2(-3.3, 3.4);
    vec2 p2 = vec2(70, -30) + iTime * vec2(3.9, -3.4);
    vec2 p3 = vec2(70, 60) + iTime * vec2(3.6, 3.2);

    q = p - coe;
    HitInfo earth = HitInfo(EARTH, sphereSDF(q / iEarthRadius) * iEarthRadius);

    earth = ground(earth, p, p1);
    earth = ground(earth, p, p2);
    earth = ground(earth, p, p3);

    earth = mushroom(earth, p, p1);
    earth = mushroom(earth, p, p2);
    earth = mushroom(earth, p, p3);

    float sky_radius = 2 * iEarthRadius;
    HitInfo sky = HitInfo(SKY, - sphereSDF(p / sky_radius) * sky_radius);

    return unionH(sky, earth);

}

float sceneSDF(const in vec3 samplePoint) {
    HitInfo hit = sceneH(samplePoint);
    return hit.dist;
}

HitInfo raymarch(const in Ray ray, float start, float end) {
    float depth = start;
    HitInfo hit;
    for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
        hit = sceneH(ray.eye + depth * ray.dir);
        if (hit.dist < EPSILON) {
            return HitInfo(hit.object, depth);
        }
        depth += hit.dist;
        if (depth >= end) {
            return HitInfo(NO_HITS, 0);
        }
    }
    return HitInfo(NO_HITS, 0);
}




vec3 estimateNormal(vec3 p) {
    // assuming we are on the surface where sceneSDF ~ 0
    return normalize(vec3(
        sceneSDF(vec3(p.x + NORMAL_DELTA, p.y, p.z)),
        sceneSDF(vec3(p.x, p.y + NORMAL_DELTA, p.z)),
        sceneSDF(vec3(p.x, p.y, p.z + NORMAL_DELTA))
    ));
}


vec3 getColorGeom(const in Light light, vec3 p, vec3 N, vec3 V, const in Material mat) {

    vec3 c = vec3(0., 0., 0.);

    vec3 L = normalize(light.pos - p);

    float d = dot(L, N);

    if (d < 0) return c;

    c += d * light.color * mat.diffuse;

    // Blinn-Phong
    vec3 H = normalize(L + V);
    float k = dot(H, N);
    if (k < 0) return c;

    c += pow(k, mat.shininess) * light.color * mat.specular;

    return c;
}



Material getMaterial(int object, vec3 p, vec3 n) {
    Material mat;
    mat.ambient = vec3(.1, .1, .1);
    mat.specular = vec3(.6, .6, .6);
    mat.shininess = 10;
    if (object == THING) {
        mat.diffuse = square(1, 2, p.x * 10) * square(0.5, 1, p.z * 10) * vec3(.2, .9, .1);
    } else if (object == SKY) {
        mat.diffuse = vec3(.1, .1, .3);
    } else if (object == EARTH) {
        mat.diffuse = square(.5, 1.4, p.x) * square(.5, .7, p.y) * vec3(.56, .09, .03);
    } else {
        mat.diffuse = vec3(.0, .0, .0);
    }

    return mat;
}

vec3 getSkyGradient(const in vec3 dir)
{
    const vec3 sky = 0.15 * vec3(13./255, 214./255, 208./255);
    float blend = dir.z * 0.5 + 0.5;
    return mix(vec3(.0), sky, blend);
}

// use distance field to evaluate ambient occlusion
float getAmbientOcclusion(const in vec3 p, const in vec3 n)
{
    float occl = 1.;
    float dist = 0.;

    for (int i = 0; i < 5; i++) {
        dist += .1;
        float sdist = sceneSDF(p + n * dist);
        occl *= 1. - max(0., (dist - sdist) * .4 / dist);
    }
    return occl;
}

float getShadow(const in vec3 l, const in vec3 p) {
    float dist = length(p - l);

    Ray ray;
    ray.dir = (p - l) / dist;
    ray.eye = l;

    HitInfo hit = raymarch(ray, MIN_DIST, MAX_DIST);
    if (hit.object == NO_HITS) {
        return 0.;
    }

    return mix(.2, 1., step(dist - 0.01, hit.dist));
}



vec3 getColor(const in Ray ray, const in HitInfo hit) {

    if (hit.object == SKY) {
        vec3 c = getSkyGradient(ray.dir);
        return pow(10 * c, vec3(1.8));
    }

    Light light;
    vec3 baseColor = vec3(.8);

    vec3 p = ray.eye + hit.dist * ray.dir;
    vec3 n = estimateNormal(p);

    Material mat = getMaterial(hit.object, p, n);

    float ambientOcclusion = getAmbientOcclusion(p, n);
    vec3 c = getSkyGradient(n) * ambientOcclusion;

    c += getSkyGradient(reflect(ray.dir, n)) * ambientOcclusion;

    // light.pos = vec3(4*sin(iTime), 4*cos(iTime), 2);
    // c += getColorGeom(light, p, n, -ray.dir, mat);

    light.pos = vec3(4*sin(0.37 * iTime), 4*cos(0.37 * iTime), 20);
    light.color = getShadow(light.pos, p) * baseColor;
    c += getColorGeom(light, p, n, -ray.dir, mat);

    return c;
}


void mainImage(out vec4 fragColor, in vec2 fragCoord) {

    vec4 p1 = vec4(2 * fragCoord / iResolution.xy - 1, 1, 1);
    vec3 p2 = (iWindowToView * p1).xyz;

    Ray ray;
    ray.dir = normalize((iViewToWorld * vec4(p2, .0)).xyz);
    ray.eye = iViewToWorld[3].xyz;

    HitInfo hit = raymarch(ray, MIN_DIST, MAX_DIST);

    if (hit.object == NO_HITS) {
        // Didn't hit anything
        fragColor = vec4(0., 0., .0, .0);
        return;
    }

    fragColor = vec4(getColor(ray, hit), 1);
}
