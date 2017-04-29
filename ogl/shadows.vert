#version 120
attribute vec4 vertex;
uniform mat4 pv_matrix;
uniform mat4 m_matrix;
uniform vec4 light_direction;
uniform vec4 plane_normal;
uniform vec4 point_in_plane;

void main(void) {
    // 1 - alpha / alpha blending
    gl_FrontColor = vec4(0, 0, 0, 0.3);

    vec3 L = normalize(light_direction.xyz);
    vec3 N = normalize(plane_normal.xyz);
    vec3 X = point_in_plane.xyz;
    float D = 1 / dot(L, N);
    mat3 R = mat3(1.0) - D * outerProduct(L, N);
    vec3 T = dot(X, N) * D * L;
    mat4 shadow_matrix = mat4(R[0], 0, R[1], 0, R[2], 0, T, 1);
    gl_Position = pv_matrix * shadow_matrix * m_matrix * vertex;
}
