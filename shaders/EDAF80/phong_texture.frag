#version 410


uniform vec3 ambient_colour; 
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;

uniform float shininess_value;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;
uniform sampler2D normal_map;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;

uniform bool use_normal_mapping;
uniform bool use_texture;

in VS_OUT{
	vec3 world_camera;	// View vector (from VS) fv
	vec3 world_normal;	// Normal (from vertex shader) fn
	vec3 world_light;   // Light vector (from VS) fl
	vec3 world_vertex;
	vec2 texcoords;
	mat3 TBN;
} vs_in;



out vec4 fColor;

void main()
{
	vec2 tex = vs_in.texcoords;
	
	vec3 new_normal; 

	if(use_normal_mapping){
		vec3 normal_map_normal = texture(normal_map, tex).rgb;
		normal_map_normal = normalize(normal_map_normal * 2.0 - 1.0);       //convert back to -1 to 1
		vec3 tnbed = vs_in.TBN * normal_map_normal;
		normal_map_normal = normalize((normal_model_to_world * vec4(tnbed, 1)).xyz);
		new_normal = normal_map_normal;
	}else{
	new_normal = vs_in.world_normal;
	}

	
	
	vec3 N = normalize(new_normal);
	vec3 L = normalize(vs_in.world_light);
	vec3 V = normalize(vs_in.world_camera);
	vec3 R = normalize(reflect(-L,N));
	
	vec3 diffuse = diffuse_colour*max(dot(N,L),0.0);
	vec3 specular = specular_colour*pow(max(dot(R,V),0.0), shininess_value);


	if(use_texture){
		vec3 my_diffuse = texture(diffuse_texture, tex).rgb;
		vec3 my_specular = texture(specular_texture, tex).rgb;
		fColor.xyz = ambient_colour + diffuse*my_diffuse + specular*my_specular;
	}else{
	fColor.xyz = ambient_colour + diffuse + specular;
	}

	fColor.w = 1.0;
}
