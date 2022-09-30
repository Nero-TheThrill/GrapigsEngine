#version 450 core

layout (location = 0) in vec4 aPos;

layout (location = 1) in vec4 test1;

layout (location = 2) in vec4 test2;


uniform mat4 model;

layout(std140, binding = 0) uniform Transform
{
	mat4 vp;
} trans;

void main()
{
	gl_Position = trans.vp *model* aPos;
}