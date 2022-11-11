#version 460 core

layout (location=0) in vec4 vPosition;
layout (location=1) in vec4 vNormal;
layout (location=2) in vec4 fNormal;
layout (location=3) in vec2 vTexCoord;

layout (location=0) out vec3 normal;
layout (location=1) out vec3 position;
layout (location=2) out vec2 texcoord;
layout (location=3) out vec3 localpos;

layout (std140, binding=0) uniform Transform
{
    mat4 worldToCamera;
    mat4 cameraToNDC;
    mat4 worldToNDC;
	vec3 camPosition;
    float camNear;
    float camFar;
} u_trans;

uniform mat4 u_modelToWorld;
uniform mat4 u_localToModel;

uniform bool u_has_normalmap;
uniform sampler2D t_normal;

void main()
{
    if(false)//u_has_normalmap)
    {   //TODO: normalmapping
        //normal = normalize(  u_modelToWorld * u_localToModel * ((texture2D(t_normal, vTexCoord))*vec4(2.0)-vec4(1.0)) ).xyz;
    }
    else
    {
        normal = vec4(normalize(u_modelToWorld * u_localToModel * vNormal)).xyz;
    }
    vec4 pos = u_modelToWorld * u_localToModel * vPosition;
	position = pos.xyz;
    texcoord = vTexCoord;
    localpos = vec4(u_localToModel * vPosition).xyz;
    gl_Position = u_trans.worldToNDC * pos;
}