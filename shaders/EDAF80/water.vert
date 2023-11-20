#version 410

layout (location = 0) in vec3 vertex;
layout (location = 2) in vec3 texcoords;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform vec3 camera_position;
uniform float elapsed_time_s;

uniform float amplitudes[2];
uniform float frequencies[2];
uniform float phases[2];
uniform float sharpness[2];
uniform vec2 directions[2];


out VS_OUT {
	vec3 vertex; 
	vec3 normal_water;
	vec3 tangent_water;
	vec3 binormal_water;
	vec2 texcoord;
} vs_out;

void waveFunction(in float A, in vec2 D, in float f, in float p, in float k, out float G,out float dGdx,out float dGdz)
{
	float angle = (D.x*vertex.x+D.y*vertex.z)*f+elapsed_time_s*p;
	float at = (sin(angle) * 0.5 + 0.5);
	float dG = 0.5*k*f*A*pow(at, k-1)*cos(angle);

	G =  A * pow(at, k);
	dGdx = dG*D.x;
	dGdz = dG*D.y;
}

void main()
{
	float G1;
	float G2;
	float dG1dx;
	float dG2dx;
	float dG1dz;
	float dG2dz;

	waveFunction(amplitudes[0],directions[0],frequencies[0],phases[0],sharpness[0],G1,dG1dx,dG1dz);
	waveFunction(amplitudes[1],directions[1],frequencies[1],phases[1],sharpness[1],G2,dG2dx,dG2dz);

	vec3 displaced_vertex = vertex;
	displaced_vertex.y += G1+G2;

	float dHdx = dG1dx+dG2dx;
	float dHdz = dG1dz+dG2dz;

	vs_out.normal_water = vec3(-dHdx, 1.0f, -dHdz);
	vs_out.binormal_water = vec3(0.0, dHdz, 1.0);
	vs_out.tangent_water = vec3(1.0, dHdx, 0.0);
    vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0)); 

	vs_out.texcoord = texcoords.xz; 
    
    gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(displaced_vertex, 1.0); 
}
