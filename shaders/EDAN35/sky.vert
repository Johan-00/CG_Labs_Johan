#version 450

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;

layout(location = 0) smooth out vec3 N;
layout(location = 1) out vec3 S;

uniform mat4 vertex_model_to_world;
uniform mat4 vertex_world_to_clip;

uniform vec3 camera_position;
uniform vec3 light_position;

void main() {
	N = normal;
	S = light_position - camera_position;
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
}
