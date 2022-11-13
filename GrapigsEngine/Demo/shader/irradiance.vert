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
    normal = (u_localToModel*u_localToModel*vNormal).xyz;
    pos = vec4( u_localToModel*vPosition).xyz;
    gl_Position =  vec4(u_trans.cameraToNDC *u_trans.worldToCamera *vec4(pos,1.0)).xyzw; 
}