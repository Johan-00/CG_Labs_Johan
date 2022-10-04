#version 410

uniform vec3 light_position;
uniform samplerCube my_reflection_cube;
uniform sampler2D my_ripple;
uniform float elapsed_time_s;

in VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec3 world_camera;
	vec3 texcoords;
	vec3 tangent;
	vec3 binormal;
} fs_in;

out vec4 frag_color;

void main()
{

	vec3 L = normalize(light_position - fs_in.vertex);
	vec3 N = normalize(fs_in.normal);
	vec3 b = normalize(fs_in.binormal);
	vec3 t = normalize(fs_in.tangent);

	mat3 BTN = mat3(b,t,N);
	
	vec3 V = normalize(fs_in.world_camera);
	
	vec4 color_deep = vec4(0.0,0.0,0.1,1.0);
	vec4 color_shallow = vec4(0.0,0.5,0.5,1.0);
	
	vec2 texScale = vec2(8, 4);
	float bumpTime = mod(elapsed_time_s, 100.0);
	vec2 bumpSpeed = vec2(-0.05, 0);
	
	vec2 bumpCoord0 = fs_in.texcoords.xz*texScale + bumpTime*bumpSpeed;
	vec2 bumpCoord1 = fs_in.texcoords.xz*texScale*2 + bumpTime*bumpSpeed*4;
	vec2 bumpCoord2 = fs_in.texcoords.xz*texScale*4 + bumpTime*bumpSpeed*8;
	
	vec4 ni0 = texture(my_ripple,bumpCoord0)*2-1;
	vec4 ni1 = texture(my_ripple,bumpCoord1)*2-1;
	vec4 ni2 = texture(my_ripple,bumpCoord2)*2-1;
	vec4 nbump_tangent_space = normalize(ni0+ni1+ni2);
	vec3 nbump = BTN*nbump_tangent_space.xyz;
	
	
	float n1 = 1.33f; // n_water, air -> water
	float n2 = 1.0f; //n_air, water -> air
	float R0 = pow((n1-n2)/(n1+n2),2);
	
	float fresnel = R0 + (1-R0)*pow(1-max(0.0,dot(V,nbump)),5);
	
	
	vec3 R = normalize(reflect(-V,nbump));
	vec4 reflection = texture(my_reflection_cube,R);
	
	vec3 r = refract(-V, nbump, n1/n2);
	vec4 refraction = texture(my_reflection_cube,r);
	
	
	float facing = 1 - max(dot(V,nbump),0.0);
	vec4 color_water = mix(color_deep,color_shallow,facing);
	frag_color = (color_water + reflection*fresnel + refraction*(1-fresnel));
	//frag_color = (color_water );
}
