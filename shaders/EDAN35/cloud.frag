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

//light parameter
uniform	int numStepsLight;
uniform	float lightAbsorbtionTowardSun;
uniform	float darknessThreshold;
uniform	float lightAbsorbtionThroughCloud = 0.94;
uniform	float phaseVal = 0.74;

//depth
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
//	Classic Perlin 3D Noise 
//	by Stefan Gustavson
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
vec4 fade(vec4 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

float cnoise(vec4 P){
  vec4 Pi0 = floor(P); // Integer part for indexing
  vec4 Pi1 = Pi0 + 1.0; // Integer part + 1
  Pi0 = mod(Pi0, 289.0);
  Pi1 = mod(Pi1, 289.0);
  vec4 Pf0 = fract(P); // Fractional part for interpolation
  vec4 Pf1 = Pf0 - 1.0; // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = vec4(Pi0.zzzz);
  vec4 iz1 = vec4(Pi1.zzzz);
  vec4 iw0 = vec4(Pi0.wwww);
  vec4 iw1 = vec4(Pi1.wwww);

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);
  vec4 ixy00 = permute(ixy0 + iw0);
  vec4 ixy01 = permute(ixy0 + iw1);
  vec4 ixy10 = permute(ixy1 + iw0);
  vec4 ixy11 = permute(ixy1 + iw1);

  vec4 gx00 = ixy00 / 7.0;
  vec4 gy00 = floor(gx00) / 7.0;
  vec4 gz00 = floor(gy00) / 6.0;
  gx00 = fract(gx00) - 0.5;
  gy00 = fract(gy00) - 0.5;
  gz00 = fract(gz00) - 0.5;
  vec4 gw00 = vec4(0.75) - abs(gx00) - abs(gy00) - abs(gz00);
  vec4 sw00 = step(gw00, vec4(0.0));
  gx00 -= sw00 * (step(0.0, gx00) - 0.5);
  gy00 -= sw00 * (step(0.0, gy00) - 0.5);

  vec4 gx01 = ixy01 / 7.0;
  vec4 gy01 = floor(gx01) / 7.0;
  vec4 gz01 = floor(gy01) / 6.0;
  gx01 = fract(gx01) - 0.5;
  gy01 = fract(gy01) - 0.5;
  gz01 = fract(gz01) - 0.5;
  vec4 gw01 = vec4(0.75) - abs(gx01) - abs(gy01) - abs(gz01);
  vec4 sw01 = step(gw01, vec4(0.0));
  gx01 -= sw01 * (step(0.0, gx01) - 0.5);
  gy01 -= sw01 * (step(0.0, gy01) - 0.5);

  vec4 gx10 = ixy10 / 7.0;
  vec4 gy10 = floor(gx10) / 7.0;
  vec4 gz10 = floor(gy10) / 6.0;
  gx10 = fract(gx10) - 0.5;
  gy10 = fract(gy10) - 0.5;
  gz10 = fract(gz10) - 0.5;
  vec4 gw10 = vec4(0.75) - abs(gx10) - abs(gy10) - abs(gz10);
  vec4 sw10 = step(gw10, vec4(0.0));
  gx10 -= sw10 * (step(0.0, gx10) - 0.5);
  gy10 -= sw10 * (step(0.0, gy10) - 0.5);

  vec4 gx11 = ixy11 / 7.0;
  vec4 gy11 = floor(gx11) / 7.0;
  vec4 gz11 = floor(gy11) / 6.0;
  gx11 = fract(gx11) - 0.5;
  gy11 = fract(gy11) - 0.5;
  gz11 = fract(gz11) - 0.5;
  vec4 gw11 = vec4(0.75) - abs(gx11) - abs(gy11) - abs(gz11);
  vec4 sw11 = step(gw11, vec4(0.0));
  gx11 -= sw11 * (step(0.0, gx11) - 0.5);
  gy11 -= sw11 * (step(0.0, gy11) - 0.5);

  vec4 g0000 = vec4(gx00.x,gy00.x,gz00.x,gw00.x);
  vec4 g1000 = vec4(gx00.y,gy00.y,gz00.y,gw00.y);
  vec4 g0100 = vec4(gx00.z,gy00.z,gz00.z,gw00.z);
  vec4 g1100 = vec4(gx00.w,gy00.w,gz00.w,gw00.w);
  vec4 g0010 = vec4(gx10.x,gy10.x,gz10.x,gw10.x);
  vec4 g1010 = vec4(gx10.y,gy10.y,gz10.y,gw10.y);
  vec4 g0110 = vec4(gx10.z,gy10.z,gz10.z,gw10.z);
  vec4 g1110 = vec4(gx10.w,gy10.w,gz10.w,gw10.w);
  vec4 g0001 = vec4(gx01.x,gy01.x,gz01.x,gw01.x);
  vec4 g1001 = vec4(gx01.y,gy01.y,gz01.y,gw01.y);
  vec4 g0101 = vec4(gx01.z,gy01.z,gz01.z,gw01.z);
  vec4 g1101 = vec4(gx01.w,gy01.w,gz01.w,gw01.w);
  vec4 g0011 = vec4(gx11.x,gy11.x,gz11.x,gw11.x);
  vec4 g1011 = vec4(gx11.y,gy11.y,gz11.y,gw11.y);
  vec4 g0111 = vec4(gx11.z,gy11.z,gz11.z,gw11.z);
  vec4 g1111 = vec4(gx11.w,gy11.w,gz11.w,gw11.w);

  vec4 norm00 = taylorInvSqrt(vec4(dot(g0000, g0000), dot(g0100, g0100), dot(g1000, g1000), dot(g1100, g1100)));
  g0000 *= norm00.x;
  g0100 *= norm00.y;
  g1000 *= norm00.z;
  g1100 *= norm00.w;

  vec4 norm01 = taylorInvSqrt(vec4(dot(g0001, g0001), dot(g0101, g0101), dot(g1001, g1001), dot(g1101, g1101)));
  g0001 *= norm01.x;
  g0101 *= norm01.y;
  g1001 *= norm01.z;
  g1101 *= norm01.w;

  vec4 norm10 = taylorInvSqrt(vec4(dot(g0010, g0010), dot(g0110, g0110), dot(g1010, g1010), dot(g1110, g1110)));
  g0010 *= norm10.x;
  g0110 *= norm10.y;
  g1010 *= norm10.z;
  g1110 *= norm10.w;

  vec4 norm11 = taylorInvSqrt(vec4(dot(g0011, g0011), dot(g0111, g0111), dot(g1011, g1011), dot(g1111, g1111)));
  g0011 *= norm11.x;
  g0111 *= norm11.y;
  g1011 *= norm11.z;
  g1111 *= norm11.w;

  float n0000 = dot(g0000, Pf0);
  float n1000 = dot(g1000, vec4(Pf1.x, Pf0.yzw));
  float n0100 = dot(g0100, vec4(Pf0.x, Pf1.y, Pf0.zw));
  float n1100 = dot(g1100, vec4(Pf1.xy, Pf0.zw));
  float n0010 = dot(g0010, vec4(Pf0.xy, Pf1.z, Pf0.w));
  float n1010 = dot(g1010, vec4(Pf1.x, Pf0.y, Pf1.z, Pf0.w));
  float n0110 = dot(g0110, vec4(Pf0.x, Pf1.yz, Pf0.w));
  float n1110 = dot(g1110, vec4(Pf1.xyz, Pf0.w));
  float n0001 = dot(g0001, vec4(Pf0.xyz, Pf1.w));
  float n1001 = dot(g1001, vec4(Pf1.x, Pf0.yz, Pf1.w));
  float n0101 = dot(g0101, vec4(Pf0.x, Pf1.y, Pf0.z, Pf1.w));
  float n1101 = dot(g1101, vec4(Pf1.xy, Pf0.z, Pf1.w));
  float n0011 = dot(g0011, vec4(Pf0.xy, Pf1.zw));
  float n1011 = dot(g1011, vec4(Pf1.x, Pf0.y, Pf1.zw));
  float n0111 = dot(g0111, vec4(Pf0.x, Pf1.yzw));
  float n1111 = dot(g1111, Pf1);

  vec4 fade_xyzw = fade(Pf0);
  vec4 n_0w = mix(vec4(n0000, n1000, n0100, n1100), vec4(n0001, n1001, n0101, n1101), fade_xyzw.w);
  vec4 n_1w = mix(vec4(n0010, n1010, n0110, n1110), vec4(n0011, n1011, n0111, n1111), fade_xyzw.w);
  vec4 n_zw = mix(n_0w, n_1w, fade_xyzw.z);
  vec2 n_yzw = mix(n_zw.xy, n_zw.zw, fade_xyzw.y);
  float n_xyzw = mix(n_yzw.x, n_yzw.y, fade_xyzw.x);
  return 2.2 * n_xyzw;
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
	float d = cnoise(vec4(pos*cloudDetailScale,1.0f));
	d = max(0,d)*cloudDetailMultiplier;
	c = max(0,c-cloudDensityThreshold)*cloudDensityMultiplier;
	float min_dist = min_distance_to_cuboid(pos);
	float cloudDistanceFade = smoothstep(0,300,min_dist);
	return (c*(1-d))*cloudDistanceFade;
	}

float lightmarch(vec3 p){
	//calculate direction of ligt form p
	vec3 dirToLight = normalize(vs_in.world_light-p);
	float dstInsideBox = rayBoxDst(BoundsMin, BoundsMax, p, dirToLight).y;

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
	if(dstInsideBox > 0.2 && (dstToBox<linearDepth||(linearDepth>mFar-250.0f))){
		float stepSize = dstInsideBox/cloudSampleCount;
		float dstLimit = dstInsideBox+dstToBox;
		float dstTravelled = dstToBox; 

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
