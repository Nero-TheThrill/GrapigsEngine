#version 460 core

layout (location=0) in vec3 inPosition;

layout (std140, binding=0) uniform Transform
{
    mat4 worldToCamera;
    mat4 cameraToNDC;
    mat4 worldToNDC;
	vec3 camPosition;
    float camNear;
    float camFar;
} u_trans;

void main()
{
    gl_Position = u_trans.worldToNDC * vec4(inPosition, 1);
}