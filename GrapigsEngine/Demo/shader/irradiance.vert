#version 460 core

layout (location=0) in vec4 vPosition;
layout (location=1) in vec4 vNormal;

layout (location=0) out vec3 normal;
layout (location=1) out vec3 pos;

layout (std140, binding=0) uniform Transform
{
    mat4 worldToCamera;
    mat4 cameraToNDC;
    mat4 worldToNDC;
	vec3 camPosition;
    float camNear;
    float camFar;
} u_trans;

uniform mat4 u_localToModel;
uniform mat4 u_modelToWorld;
void main()
{
    normal = vNormal.xyz;
    pos = vPosition.xyz;
    gl_Position =  u_trans.worldToNDC * u_modelToWorld * u_localToModel * vPosition;
}