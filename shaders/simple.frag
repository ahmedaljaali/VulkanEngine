#version 450

// Out variables
layout(location = 0) out vec4 outColor;

// in variables
layout(location = 0) in vec3 inFragColor;

layout(push_constant) uniform Push
{
    mat4 transform;
    vec3 color;
} push;

void main(void)
{
    outColor = vec4(normalize(inFragColor), 1.0F);
}
