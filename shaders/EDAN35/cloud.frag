#version 410
//cloud shape
uniform vec3 BoundsMin;
uniform vec3 BoundsMax;
uniform vec3 cloudOffset;
uniform float cloudScale;
uniform float cloudDensityThreshold;
uniform float cloudDensityMultiplier;
uniform int cloudSampleCount;
uniform float elapsed_time_s;

//detail cloud
uniform float cloudDetailScale;
uniform float cloudDetailMultiplier;

uniform sampler2D depthTexture;
uniform float mNear;
uniform float mFar;

uniform vec2 view_port;
uniform mat4 inv_proj;
uniform mat4 inv_view;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;


in VS_OUT{
	vec3 world_camera;	// View vector (from VS) fv
	vec3 world_camera_pos;
	vec3 world_normal;	// Normal (from vertex shader) fn
	vec3 world_light;   // Light vector (from VS) fl
	vec3 world_vertex;
	vec2 texcoords;
	vec2 view_port;
	mat3 TBN;
} vs_in;



out vec4 fColor;


float r(vec3 n)
{
 	return fract(cos(n.x*89.42+n.y*23.62+n.z*45.13)*343.42);
}

float worley(vec3 n,float s)
{
	float dis = 2.0;
	for(int x = -1;x<=1;x++)
	{
		for(int y = -1;y<=1;y++)
		{
			for(int z = -1;z<=1;z++)
			{
				vec3 p = floor(n/s)+vec3(x,y,z);
				float d = length(r(p)+vec3(x,y,z)-fract(n/s));
				if (dis>d)
				{
					dis = d;   
				}
			}
		}
	}
	return dis;
}
float PerlinNoise3D(vec3 p){
	vec3 pi = floor(p);
	vec3 pf = p - pi;
	vec3 w = pf * pf * (3.0 - 2.0 * pf);
	float f000 = r(pi + vec3(0.0, 0.0, 0.0));
	float f100 = r(pi + vec3(1.0, 0.0, 0.0));
	float f010 = r(pi + vec3(0.0, 1.0, 0.0));
	float f110 = r(pi + vec3(1.0, 1.0, 0.0));
	float f001 = r(pi + vec3(0.0, 0.0, 1.0));
	float f101 = r(pi + vec3(1.0, 0.0, 1.0));
	float f011 = r(pi + vec3(0.0, 1.0, 1.0));
	float f111 = r(pi + vec3(1.0, 1.0, 1.0));
	float x00 = mix(f000, f100, w.x);
	float x10 = mix(f010, f110, w.x);
	float x01 = mix(f001, f101, w.x);
	float x11 = mix(f011, f111, w.x);
	float y0 = mix(x00, x10, w.y);
	float y1 = mix(x01, x11, w.y);
	float z = mix(y0, y1, w.z);
	return z;
	}

vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 rayDir){
	vec3 t0s = (boundsMin - rayOrigin) / rayDir;
	vec3 t1s = (boundsMax - rayOrigin) / rayDir;

	vec3 tmin = min(t0s, t1s);
	vec3 tmax = max(t0s, t1s);

	float dstA = max(max(tmin.x, tmin.y), tmin.z);
	float dstB = min(min(tmax.x, tmax.y), tmax.z);

	float dstToBox = max(0, dstA);
	float dstInsideBox = max(0,dstB - dstToBox);

	return vec2(dstToBox, dstInsideBox);
	}

// Function to calculate the minimum distance from a point to a cuboid
float min_distance_to_cuboid(vec3 p) {
    float dx = min(abs(p.x - BoundsMin.x), abs(p.x - BoundsMax.x));
    //float dy = min(abs(p.y - BoundsMin.y), abs(p.y - BoundsMax.y));
    float dz = min(abs(p.z - BoundsMin.z), abs(p.z - BoundsMax.z));

    return min(dx, dz);
}

float sampleDensety(vec3 pos){
	pos = pos + cloudOffset*elapsed_time_s;
	vec3 p = pos*0.01*cloudScale;
	float c = 1-worley(p,1.0f);
	float d = PerlinNoise3D(pos*cloudDetailScale);
	d = max(0,d)*cloudDetailMultiplier;
	c = max(0,c-cloudDensityThreshold)*cloudDensityMultiplier;
	float min_dist = min_distance_to_cuboid(pos);
	float cloudDistanceFade = smoothstep(0,60,min_dist);
	return (c*(1-d))*cloudDistanceFade;
	}

float lightmarch(vec3 p){
	vec3 dirToLight = normalize(vs_in.world_light);
	float dstInsideBox = rayBoxDst(BoundsMin, BoundsMax, p, dirToLight).y;


	int numStepsLight = 5;
	float lightAbsorbtionTowardSun = 0.85;
	float darknessThreshold = 0.07;

	float stepSize = dstInsideBox/numStepsLight;
	float totalDensity = 0;

	for(int i = 0;i<numStepsLight;i++){
		 vec3 pos = p + dirToLight * stepSize * i;
		totalDensity += max(0,sampleDensety(pos)*stepSize);
		}
	float transmittance = exp(-totalDensity*lightAbsorbtionTowardSun);
	
	return darknessThreshold + transmittance * (1-darknessThreshold);
	}

void main()
{
	vec2 texcoords = vec2(gl_FragCoord.x/view_port.x,gl_FragCoord.y/view_port.y);
	float depth = (texture(depthTexture, texcoords).x);
	float linearDepth = (2.0 * mNear * mFar) / (mFar + mNear - depth * (mFar - mNear));
	//float linearDepth = (2.0 * mNear) / (mFar + mNear - depth * (mFar - mNear));
	// remap depf using camra view matrix




	float x = 2.0 * gl_FragCoord.x / view_port.x - 1.0;
	float y = 2.0 * gl_FragCoord.y / view_port.y - 1.0;
	vec2 ray_nds = vec2(x, y);
	vec4 ray_clip = vec4(ray_nds, -1.0, 1.0);
	vec4 ray_view = inv_proj * ray_clip;
	ray_view = vec4(ray_view.xy, -1.0, 0.0);
	vec3 ray_world = (inv_view * ray_view).xyz;
	ray_world = normalize(ray_world);
	vec3 rayPos = vec3(0.0f,0.0f,0.0f);

	vec3 rayOrigin = vs_in.world_camera_pos;
	vec2 t = rayBoxDst(BoundsMin, BoundsMax, rayOrigin, ray_world);
	float dstToBox = t.x;
	float dstInsideBox = t.y;
	if(dstInsideBox > 0.2 && dstToBox<linearDepth){
		float stepSize = dstInsideBox/cloudSampleCount;
		float dstLimit = dstInsideBox+dstToBox;
		float dstTravelled = dstToBox; 
		
	
		float lightAbsorbtionThroughCloud = 0.94;
		//float lightTransmittance = 0.5;
		float phaseVal = 0.74;

		//march through volume
		float transmittance = 1;
		float lightEnergy = 0;

		while(dstTravelled <dstLimit){
			rayPos = rayOrigin+ray_world*dstTravelled;
			float density = sampleDensety(rayPos);
			if(density > 0.01){
				float light = lightmarch(rayPos);
				lightEnergy += density*stepSize*transmittance*light*phaseVal;
				transmittance *= exp(-density*stepSize*lightAbsorbtionThroughCloud);
				if(transmittance < 0.01){
					break;
				}
		
			}
			dstTravelled += stepSize;
		}
		vec3 lightColor = vec3(1.0f,1.0f,1.0f);
		vec3 color = lightColor*lightEnergy;
		fColor = vec4(color,(1-transmittance));

	}
	else{
		fColor = vec4(0.0f,0.0f,0.0f,0.0f);


		}

}
