attribute vec3 vertex;
uniform mat4 model_matrix;

void main(void) {
	gl_Position.w = 1.;
  gl_Position.xyz = (model_matrix * vec4(vertex, 1)).xyz;
}
