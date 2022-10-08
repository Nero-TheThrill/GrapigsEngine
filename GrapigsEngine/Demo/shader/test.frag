#version 460 core

layout (location=0) in vec3 normal;
layout (location=1) in vec3 position;
layout (location=2) in vec2 texcoord;
layout (location=0) out vec4 output_color;

uniform vec4 u_color;
uniform float o_ambient;
uniform float o_diffuse;
uniform float o_specular;
uniform sampler2D u_texture;

vec3 I_a, I_d, I_s;

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

vec3 objcolor = texture(u_texture,texcoord).xyz;

vec3 PointLight(Light light)
{	
	I_a = objcolor * o_ambient * light.ambient;

	vec3 lightVector = normalize(light.position - position);

	I_d = o_diffuse * light.diffuse * max(dot(normal, lightVector), 0.0);

	vec3 viewDirection = normalize(u_trans.camPosition - position);
	vec3 reflectDirection = 2 * dot(normal, lightVector) * normal - lightVector;

	if(dot(normal, lightVector) > 0.0)
	{
		I_s = o_specular * light.specular * pow(max(dot(viewDirection, reflectDirection), 0.0), 32); 
	}
	else
		I_s = vec3(0, 0, 0);
		
	float lightLength = length(light.position - position);
	float attenuation=min(1 / (light.c.x + light.c.y * lightLength + light.c.z * lightLength * lightLength), 1);
	vec3 I_local = attenuation * (I_a + I_d + I_s);
	return I_local;
}

vec3 DirectionalLight(Light light)
{	
	I_a = objcolor * o_ambient * light.ambient;

	vec3 lightVector = normalize(-light.direction);

	I_d = o_diffuse * light.diffuse * max(dot(normal, lightVector),0.0);

	vec3 viewDirection=normalize(u_trans.camPosition - position);
	vec3 reflectDirection = 2 * dot(normal, lightVector) * normal - lightVector;

	if(dot(normal, lightVector) > 0.0)
	{
		I_s = o_specular * light.specular * pow(max(dot(viewDirection, reflectDirection), 0.0), 32); 
	}
	else
		I_s = vec3(0, 0, 0);

	vec3 I_local = (I_a + I_d + I_s);
	return I_local;
}

vec3 SpecularLight(Light light)
{	
	I_a = objcolor * o_ambient * light.ambient;

	vec3 lightVector = normalize(light.position - position);

	I_d = o_diffuse * light.diffuse * max(dot(normal, lightVector),0.0);

	vec3 viewDirection = normalize(u_trans.camPosition - position);
	vec3 reflectDirection = 2 * dot(normal, lightVector) * normal - lightVector;

	if(dot(normal, lightVector) > 0.0)
	{
		I_s = o_specular * light.specular * pow(max(dot(viewDirection, reflectDirection), 0.0), 32); 
	}
	else
		I_s = vec3(0, 0, 0);

	float alpha = dot(-lightVector, normalize(light.direction)); 
    float spotlighteffect = 0;
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
	float light_length = length(light.position - position);
	float attenuation = min(1 / (light.c.x + light.c.y * light_length + light.c.z * light_length * light_length), 1);
	vec3 I_local = attenuation * spotlighteffect * (I_d + I_s) + attenuation * I_a;
	return I_local;
}

void main()
{
	vec3 finalColor = vec3(0);
	for(uint i=0; i < lightInfo.lightNum; i++) 
	{
		Light light = lightInfo.lights[i];
		switch(light.type)
		{
			case 0:
				finalColor += PointLight(light);
				break;
			case 1:
				finalColor += DirectionalLight(light);
				break;
			case 2:
				finalColor += SpecularLight(light);
				break;
			default:
		}

	}
	
    output_color = vec4(finalColor, 1);
}

