
#version 400 core

// Uniforms
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec4 vPosition;

out vec4 color;

void main(void)
{
    color = vColor;
    gl_Position = projection_matrix * view_matrix * model_matrix * vPosition;
}
