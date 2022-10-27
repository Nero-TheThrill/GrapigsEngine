#version 460 core

layout (location=0) in vec3 normal;
layout (location=1) in vec3 position;
layout (location=2) in vec2 texcoord;
layout (location=0) out vec4 output_color;

uniform vec4 u_color;
uniform float o_ambient;
uniform float o_diffuse;
uniform float o_specular;
uniform float o_metallic;
uniform float o_roughness;
uniform sampler2D o_albedo;

vec3 I_a, I_d, I_s;
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
vec3 fresnelSchlick(float HdotV, vec3 baseReflectivity)
{
	return baseReflectivity + (1.0 - baseReflectivity) * pow(1.0 - HdotV, 5.0);
}
//----------------------------------Lighting----------------------------------------//



vec3 CalculateFinalColor()
{
	vec3 albedo = texture2D(o_albedo, texcoord).xyz;
	vec3 finalColor = vec3(0);
	vec3 viewDirection = normalize(u_trans.camPosition - position);
	vec3 baseReflectivity = mix(vec3(0.04), albedo, o_metallic);
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

		float D = distributionGGX(NdotH, o_roughness);
		float G = geometrySmith(NdotV, NdotL, o_roughness);
		vec3 F = fresnelSchlick(HdotV, baseReflectivity);
		vec3 specular = D * G * F;
		specular /= 4.0 * NdotV * NdotL;
		vec3 kD = vec3(1.0) - F;
		kD *= 1.0 - o_metallic;

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
	}
	//finalColor += objcolor * vec3(0.15);
	//finalColor = finalColor/(finalColor+vec3(1.0));
	//finalColor = pow(finalColor, vec3(1.0/2.2)); 
	return finalColor;
}

void main()
{	

    output_color = vec4(CalculateFinalColor(), 1);
}

