#version 460 core

layout (location=0) in vec3 normal;
layout (location=1) in vec3 position;
layout (location=2) in vec2 texcoord;
layout (location=3) in vec3 localpos;
layout (location=0) out vec4 output_color;

uniform sampler2D t_irradiance;
uniform sampler2D t_brdflut;
uniform sampler2D t_ibl;
uniform samplerCube t_prefiltermap;


uniform vec3 u_albedo;
uniform float u_metallic;
uniform float u_roughness;

uniform bool u_has_albedo;
uniform sampler2D t_albedo;

uniform bool u_has_metallic;
uniform sampler2D t_metallic;

uniform bool u_has_roughness;
uniform sampler2D t_roughness;

uniform bool u_has_ao;
uniform sampler2D t_ao;

const float PI = 3.141592654;

layout (std140, binding=0) uniform Transform
{
    mat4 worldToCamera;
    mat4 cameraToNDC;
    mat4 worldToNDC;
	vec3 camPosition;
    float camNear;
    float camFar;
} u_trans;

struct Light
{
	uint type;    	   
	vec3 direction;    
	vec3 position;    
	vec3 ambient;      
	vec3 diffuse;      
	vec3 specular;     
	vec3 c;
	float inner_angle;
	float outer_angle; 
	float falloff;     
};

layout(std140, binding = 1) uniform LightInformation
{	
	uint lightNum;
	Light lights[16];
}lightInfo;


//----------------------------------PBR----------------------------------------//
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float distributionGGX(float NdotH, float roughness)
{
	float a = roughness * roughness;
	float pow_a = a * a;
	float denom = NdotH * NdotH * (pow_a - 1.0) + 1.0;
	denom = PI * denom * denom;
	return pow_a / max(denom, 0.0000001);
}
float geometrySmith(float NdotV, float NdotL, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
	float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
	return ggx1 * ggx2;
}
vec3 fresnelSchlick(float HdotV, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - HdotV, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float NdotV, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
} 
//----------------------------------Lighting----------------------------------------//



vec3 CalculateFinalColor()
{
	vec3 albedo = u_albedo;
	if(u_has_albedo)
		albedo =pow(texture2D(t_albedo, texcoord).xyz, vec3(2.2));
	float metallic = u_metallic;
	if(u_has_metallic)
		metallic = texture2D(t_metallic, texcoord).x;
	float roughness = u_roughness;
	if(u_has_roughness)
		roughness = texture2D(t_roughness, texcoord).x;
	float ao = 1.0f;
	if(u_has_ao)
		ao = texture2D(t_ao, texcoord).x;

	vec3 finalColor = vec3(0);
	vec3 viewDirection = normalize(u_trans.camPosition - position);
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	/*
	for(uint i=0; i < lightInfo.lightNum; i++) 
	{	
		Light light = lightInfo.lights[i];
		vec3 lightVector = normalize(light.position - position);
		vec3 halfwayVector = normalize(viewDirection+lightVector);
		float distance = length(light.position - position);
		float attenuation = min(1 / (light.c.x + light.c.y * distance + light.c.z * distance * distance), 1);
		vec3 radiance = vec3(0);// = light.ambient * attenuation;

		float NdotV = max(dot(normal, viewDirection),0.0000001);
		float NdotL = max(dot(normal, lightVector),0.0000001);
	    float HdotV = max(dot(halfwayVector, viewDirection),0.0);
		float NdotH = max(dot(normal, halfwayVector),0.0);

		float D = distributionGGX(NdotH, roughness);
		float G = geometrySmith(NdotV, NdotL, roughness);
		vec3 F = fresnelSchlick(HdotV, F0);
		vec3 specular = D * G * F;
		specular /= 4.0 * max(NdotV, 0.0) * max(NdotL, 0.0) + 0.0001; 

		vec3 kD = vec3(1.0) - F;
		kD *= 1.0 - metallic;

		switch(light.type)
		{
			case 0:
				radiance = light.ambient * attenuation;
				break;
			case 1:
				radiance = light.ambient;
				break;
			case 2:
			    float spotlighteffect = 0;
			    float alpha = dot(-lightVector, normalize(light.direction)); 
   				if(alpha < cos(light.outer_angle))
    			{
    				spotlighteffect = 0;
   				}
    			else if(alpha > cos(light.inner_angle))
    			{
    				spotlighteffect = 1;
    			}
    			else
    			{
    				spotlighteffect = pow((alpha - cos(light.outer_angle)) / (cos(light.inner_angle) - cos(light.outer_angle)), light.falloff);
    			}
				radiance = light.ambient * attenuation * spotlighteffect;
				break;
			default:
		}

	    finalColor += (kD * albedo / PI + specular) * radiance * NdotL;
	}*/
 	vec3 kS = fresnelSchlickRoughness(max(dot(normal, viewDirection), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;
    kD*=1.0-metallic;
	vec3 irradiance =  texture(t_irradiance, SampleSphericalMap(normalize(normal))).xyz;
 	vec3 diffuse      = irradiance * albedo;

 	vec3 R = reflect(-viewDirection, normal);
	const float MAX_REFLECTION_LOD = 6.0;
    vec3 prefilteredColor = textureLod(t_prefiltermap, R,  roughness * MAX_REFLECTION_LOD).rgb; 
    vec2 brdf  = texture(t_brdflut, vec2(max(dot(normal, viewDirection), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    finalColor+=ambient;
	finalColor = finalColor/(finalColor+vec3(1.0));
	finalColor = pow(finalColor, vec3(1.0/2.2)); 



	return finalColor;
}

void main()
{	

    output_color = vec4(CalculateFinalColor(), 1);
}

