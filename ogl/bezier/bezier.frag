#version 450 core

out vec4 color;

layout (binding = 0) uniform sampler2D tiles;

in GS_OUT {
    vec2 tex;
    float diffuse;
} fs_in;

const vec4 light_color = vec4(1., .0, .0, 1);

void main() {
  color = texture(tiles, fs_in.tex) + light_color * fs_in.diffuse;
}
