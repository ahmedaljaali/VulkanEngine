#version 450

// in variables
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

// out variables
layout(location = 0) out vec3 outFragColor;

layout(push_constant) uniform Push
{
    mat4 transform;
    vec3 color;
} push;

void main(void)
{
    gl_Position = push.transform * vec4(inPosition, 1.0F);
    outFragColor = inColor;
}
