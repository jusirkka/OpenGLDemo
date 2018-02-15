#version 420 core

layout (location = 0) out vec4 color;

in vec2 tc;

layout (binding = 0) uniform sampler2D tex;

void main(void)
{
    color = texture(tex, tc);
}
