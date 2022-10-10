#version 410

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texcoords;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 binormal;

uniform vec3 light_position;
uniform vec3 camera_position;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;


out VS_OUT{
	vec3 world_camera;
	vec3 world_normal;
	vec3 world_light;
	vec3 world_vertex;
	vec2 texcoords;
	mat3 TBN;
	} vs_out;


void main()
{
	//Construct TBN matrix for normal mapping
	vec3 T = normalize(tangent);
	vec3 B = normalize(binormal);
	vec3 N = normalize(normal);
	mat3 TBN = mat3(T, B, N);



	vs_out.world_vertex = vec3(vertex_model_to_world * vec4(vertex, 1.0));
	vs_out.world_normal = (normal_model_to_world * vec4(normal, 0.0)).xyz;
	vs_out.texcoords = texcoords.xy;
	
	vs_out.world_camera = camera_position - (vertex_model_to_world * vec4(vertex, 1.0)).xyz;
	vs_out.world_light = light_position - (vertex_model_to_world * vec4(vertex, 1.0)).xyz;
	vs_out.TBN = TBN;

	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
}
