#version 450

// in variables
layout(location = 0) out vec4 outColor;

// out variables
layout(location = 0) in vec3 fragColor;

layout(push_constant) uniform Push
{
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;

void main(void)
{
    outColor = vec4(fragColor * push.color, 1.0F);
}
