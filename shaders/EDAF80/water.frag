#version 410

uniform vec3 light_position;
uniform samplerCube my_reflection_cube;
uniform sampler2D my_ripple;
uniform vec3 deep_color;
uniform vec3 shallow_color;
uniform float elapsed_time_s;
uniform vec3 camera_position;
uniform mat4 normal_model_to_world;

in VS_OUT {
	vec3 vertex;
	vec3 normal_water;
	vec3 tangent_water;
	vec3 binormal_water;
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

void main()
{

	mat3 TBN_water = mat3(normalize(fs_in.tangent_water), normalize(fs_in.binormal_water), normalize(fs_in.normal_water));
	vec2 texScale = vec2(8, 4);
	float normalTime = mod(elapsed_time_s, 100.0);
	vec2 normalSpeed = vec2(-0.05, 0);
	vec3 normal = vec3(0.0);

	vec3 V = normalize(camera_position - fs_in.vertex);

	//normal maping
	vec2 normalCoord0 = (fs_in.texcoord.xy*texScale + normalTime*normalSpeed).xy;
	vec2 normalCoord1 = (fs_in.texcoord.xy*texScale*2 + normalTime*normalSpeed*4).xy;
	vec2 normalCoord2 = (fs_in.texcoord.xy*texScale*4 + normalTime*normalSpeed*8).xy;
	vec3 n1 = (texture(my_ripple, normalCoord0).xyz * 2 - 1);
	vec3 n2 = (texture(my_ripple, normalCoord1).xyz * 2 - 1);
	vec3 n3 = (texture(my_ripple, normalCoord2).xyz * 2 - 1);
	vec3 n_bump = normalize(n1 + n2 + n3);

	normal = (normal_model_to_world * vec4(TBN_water * n_bump, 0)).xyz;
	normal = normalize(normal);

	if (gl_FrontFacing){
		normal = -normal;
	}
	vec3 reflection = texture(my_reflection_cube, reflect(-V, normalize(normal))).xyz;

	float facing = 1 - max(dot(V, normalize(normal)),0.0);
	vec3 water_color = mix(deep_color, shallow_color, facing);

	float n_1 = 1.33f; // n_water, air -> water
	float n_2 = 1.0f; //n_air, water -> air
	float r0 = pow((n_1-n_2)/(n_1+n_2),2);

	vec3 refraction;
	if (gl_FrontFacing){
			refraction  = texture(my_reflection_cube, refract(-V,normal, n_1/n_2)).xyz;
	} else {
			refraction  = texture(my_reflection_cube, refract(-V,normal, n_2/n_1)).xyz;
	}
	float fresnel = r0 + (1-r0)*pow(1-max(0.0,dot(V,normal)),5);

	frag_color = vec4(water_color + reflection * fresnel + refraction * (1 - fresnel) ,1.0);
} 
