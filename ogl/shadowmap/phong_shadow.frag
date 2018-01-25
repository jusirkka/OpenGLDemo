#version 130
uniform  mat4 material_colors;
uniform  mat4 light_colors;
uniform float scale;

uniform sampler2D img;
uniform sampler2D shadowmap;

in vec4 cosines;
in vec2 texcoord;
in vec4 shadowcoord;

void main(void) {

 	float shadow = 1.0;
 	if (shadowcoord.w > 0.0) {
    vec4 sc_proj = shadowcoord / shadowcoord.w ;
    float distanceFromLight = texture2D(shadowmap, sc_proj.st).x;
 		shadow = distanceFromLight < sc_proj.z ? 0.5 : 1.0;
  }

    mat4 M = matrixCompMult(light_colors, material_colors);

    gl_FragColor = shadow * (texture2D(img, texcoord * scale) +  M * cosines);
}
