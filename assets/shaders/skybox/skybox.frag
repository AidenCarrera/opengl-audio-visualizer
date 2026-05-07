#version 330 core

in vec3 tex_coord;

layout (location = 0) out vec4 color;

// Texture for cube mapping
uniform samplerCube tex;

void main(void)
{
	color = texture(tex, tex_coord);
}