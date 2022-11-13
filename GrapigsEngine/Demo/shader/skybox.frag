#version 460 core

layout (location=0) in vec3 pos;

layout (location=0) out vec4 output_color;

uniform sampler2D t_environment;
const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{	

    output_color = texture(t_environment, SampleSphericalMap(normalize(pos)));
}

