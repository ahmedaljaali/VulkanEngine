#version 450

// in variables
layout(location = 0) out vec4 outColor;

// out variables
layout(location = 0) in vec3 fragColor;


void main(void)
{
    outColor = vec4(fragColor, 1.0F);
}
