#version 450

// in variables
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 inColor;

// out variables
layout(location = 0) out vec3 fragColor;

void main(void)
{
    gl_Position = vec4(position, 0.0F, 1.0F);
    fragColor = inColor;
}
