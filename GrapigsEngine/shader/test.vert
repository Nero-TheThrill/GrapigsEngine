#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;

layout(std140, binding = 0) uniform Transform
{
	mat4 vp;
} trans;

void main()
{
	gl_Position = trans.vp * vec4(aPos, 1.0);
}