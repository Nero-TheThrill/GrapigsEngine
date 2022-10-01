#version 460 core

layout (location=0) in vec3 normal;
layout (location=1) in vec3 position;
layout (location=2) in vec3 lightPos;

layout (location=0) out vec4 output_color;

uniform vec4 u_color;

vec3 GetColor()
{
	const vec3 lightdir = normalize(lightPos - position);
	const vec3 ambient = 0.1 * u_color.xyz;
	const float diff = max(dot(normal, lightdir), 0.0);
	const vec3 diffuse = diff * u_color.xyz;
	return vec3(ambient + diffuse) * u_color.xyz;
}

void main()
{
    output_color = vec4(GetColor(), 1);
}