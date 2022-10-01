#version 460 core

layout (location=0) in vec4 vPosition;
layout (location=1) in vec4 vNormal;
layout (location=2) in vec4 fNormal;

layout (location=0) out vec3 normal;
layout (location=1) out vec3 position;
layout (location=2) out vec3 lightPos;

layout (std140, binding=0) uniform Transform
{
    mat4 worldToCamera;
    mat4 cameraToNDC;
    mat4 worldToNDC;
	vec3 cameraPosition;
} u_trans;

uniform mat4 u_modelToWorld;
uniform mat4 u_localToModel;

void main()
{
    normal = normalize(fNormal.xyz);
    vec4 pos = u_modelToWorld * u_localToModel * vPosition;
	lightPos = u_trans.cameraPosition;
	position = pos.xyz;
    gl_Position = u_trans.worldToNDC * pos;
}