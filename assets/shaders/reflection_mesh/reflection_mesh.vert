#version 410 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;

// Pass normal and position data to the fragment shader
out VS_OUT {
    vec3 N;
    vec3 P;
} vs_out;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
    // Calculate world-space coordinate
    vec4 world_pos = model_matrix * position;
    vs_out.P = world_pos.xyz;

    // Calculate world-space normal 
    // (mat3 conversion handles standard rotational and scale transformations)
    vs_out.N = normalize(mat3(model_matrix) * normal);

    gl_Position = projection_matrix * view_matrix * world_pos;
}