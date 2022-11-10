#version 460 core

layout (location=0) in vec3 normal;

layout (location=0) out vec4 output_color;

uniform samplerCube t_ibl;

void main()
{	

    output_color = texture(t_ibl, normal);
}

