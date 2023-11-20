#version 410

struct ViewProjTransforms
{
	mat4 view_projection;
	mat4 view_projection_inverse;
};

layout (std140) uniform CameraViewProjTransforms
{
	ViewProjTransforms camera;
};

layout (std140) uniform LightViewProjTransforms
{
	ViewProjTransforms lights[4];
};

uniform int light_index;

uniform sampler2D depth_texture;
uniform sampler2D normal_texture;
uniform sampler2D shadow_texture;

uniform vec2 inverse_screen_resolution;

uniform vec3 camera_position;

uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 light_direction;
uniform float light_intensity;
uniform float light_angle_falloff;

layout (location = 0) out vec4 light_diffuse_contribution;
layout (location = 1) out vec4 light_specular_contribution;


void main()
{
	vec2 texcoord = vec2(gl_FragCoord.x * inverse_screen_resolution.x, gl_FragCoord.y * inverse_screen_resolution.y);
	float depth = texture(depth_texture, texcoord).x;
	vec4 pos = camera.view_projection_inverse * (2.0 * vec4(texcoord.xy, depth, 1.0f) - 1.0f);
	pos = pos / pos.w;

	
	vec3 L = normalize(light_position - pos.xyz);
	vec3 N = (2.0 * texture(normal_texture, texcoord).xyz) - 1.0f;
	vec3 V = normalize(camera_position - pos.xyz);

	// Falloff and compositio
	float r = distance(pos.xyz, light_position);
	r = clamp(light_intensity * (1 / (1 + (r * r))) * smoothstep(cos(light_angle_falloff), 1.0, dot(-L, light_direction)),0,1);

	// final color
	light_diffuse_contribution  = vec4(r * light_color * max(0.0, dot(N, L)),1.0f); 
	light_specular_contribution = vec4(r * light_color * pow(max(0.0, dot(reflect(-L, N), V)), 100),1.0f);
}
