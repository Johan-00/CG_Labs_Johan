#version 410

in VS_OUT {
	vec3 normal;
} fs_in;

uniform samplerCube cubeMap;

out vec4 frag_color;

void main()
{
	frag_color = texture(cubeMap, fs_in.normal);
}
