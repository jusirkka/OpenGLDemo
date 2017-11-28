#version 120

void mainImage(out vec4, in vec2);

void main(void) {
    mainImage(gl_FragColor, gl_FragCoord.xy);
    gl_FragColor.w = 1.;
}

