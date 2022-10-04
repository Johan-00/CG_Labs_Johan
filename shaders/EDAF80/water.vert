#version 410

layout (location = 0) in vec3 vertex;
layout (location = 2) in vec3 texcoords;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform vec3 camera_position;
uniform float elapsed_time_s;


out VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec3 world_camera;
	vec3 texcoords;
	vec3 tangent;
	vec3 binormal;
} vs_out;

void waveFunction(in float A, in vec2 D, in float f, in float p, in float k, out float G,out float dGdx,out float dGdz)
{
	float Dx = D.x;
	float Dz = D.y;

	float x = vertex.x;
	float z = vertex.z;

	float angle = (Dx*x+Dz*z)*f+elapsed_time_s*p;
	float at = (sin(angle) * 0.5 + 0.5);

	G =  A * pow(at, k);

	float dG = 0.5*k*f*A*pow(at, k-1)*cos(angle);

	dGdx = dG*Dx;
	dGdz = dG*Dz;
}

void main()
{
	vec2 amplitude = vec2(1.0, 0.5);
	float A1 = amplitude.x;
	float A2 = amplitude.y;
	vec2 dir1 = vec2(-1.0, 0.0);
	vec2 dir2 = vec2(-0.7f, 0.7f);
	vec2 frequency = vec2(0.2, 0.4);
	float f1 = frequency.x;
	float f2 = frequency.y;
	vec2 phase = vec2(0.5, 1.3);
	float p1 = phase.x;
	float p2 = phase.y;
	vec2 sharpness = vec2(2.0, 2.0);
	float k1 = sharpness.x;
	float k2 = sharpness.y;
	float x = vertex.x;
	float z = vertex.z;

	float G1;
	float G2;
	float dG1dx;
	float dG2dx;
	float dG1dz;
	float dG2dz;

	waveFunction(A1,dir1,f1,p1,k1,G1,dG1dx,dG1dz);
	waveFunction(A2,dir2,f2,p2,k2,G2,dG2dx,dG2dz);
	
	float y = vertex.y + G1 + G2;
	
	float dHdx = dG1dx + dG2dx;
	float dHdz = dG1dz + dG2dz;
	
	vs_out.vertex = vec3(vertex_model_to_world * vec4(x,y,z, 1.0));
	vs_out.normal = vec3(normal_model_to_world * vec4(vec3(-dHdx,1.0,-dHdz), 0.0));
	vs_out.texcoords = texcoords;
	vs_out.world_camera = camera_position - (vertex_model_to_world * vec4(x,y,z, 1.0)).xyz;
	
	vs_out.binormal = vec3(normal_model_to_world * vec4(vec3(1.0,dHdx,0.0), 0.0));
	vs_out.tangent = vec3(normal_model_to_world * vec4(vec3(0.0,dHdz,1.0), 0.0));
	
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(x,y,z, 1.0);

}

