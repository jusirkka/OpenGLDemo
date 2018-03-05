#version 450 core

layout(isolines, equal_spacing) in;


vec3 fn(float u, const in vec3 p0, const in vec3 p1, const in vec3 p2, const in vec3 p3) {
    float t = 1 - u;

    float B0 = t * t * t;
    float B1 = 3 * u * t * t;
    float B2 = 3 * u * u * t;
    float B3 = u * u * u;

    return B0 * p0 + B1 * p1 + B2 * p2 + B3 * p3;
}

vec3 df(float u, const in vec3 p0, const in vec3 p1, const in vec3 p2, const in vec3 p3) {
    float t = 1 - u;

    float B0 = - 3 * t * t;
    float B1 = 3 * (t * t - 2 * u * t);
    float B2 = 3 * (2 * u * t - u * u);
    float B3 = 3 * u * u;

    return B0 * p0 + B1 * p1 + B2 * p2 + B3 * p3;
}


vec3 ddf(float u, const in vec3 p0, const in vec3 p1, const in vec3 p2, const in vec3 p3) {
    float t = 1 - u;

    float B0 = 6 * t;
    float B1 = 6 * (- 2 * t + u);
    float B2 = 6 * (t - 2 * u);
    float B3 = 6 * u;

    return B0 * p0 + B1 * p1 + B2 * p2 + B3 * p3;
}

out TE_OUT {
    vec3 y0;
    vec3 x0;
    float t;
} te_out;

void main() {
    float u = gl_TessCoord.x;

    vec3 p0 = vec3(gl_in[0].gl_Position);
    vec3 p1 = vec3(gl_in[1].gl_Position);
    vec3 p2 = vec3(gl_in[2].gl_Position);
    vec3 p3 = vec3(gl_in[3].gl_Position);
    gl_Position = vec4(fn(u, p0, p1, p2, p3), 1.);
    vec3 z0 = normalize(df(u, p0, p1, p2, p3));
    vec3 dz = ddf(u, p0, p1, p2, p3);
    te_out.y0 = normalize(dz - z0 * dot(z0, dz));
    te_out.x0 = cross(te_out.y0, z0);
    te_out.t = u;
}
