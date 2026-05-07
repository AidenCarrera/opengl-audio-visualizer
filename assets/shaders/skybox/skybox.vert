#version 330 core

layout (location = 0) in vec3 in_position;

out vec3 tex_coord;

// Uniforms
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main(void)
{
	// Since the sky box represents the background, we don't consider the camera translation.
	// Therefore, we remove the translation component of the view matrix by converting it to mat3 and then back to mat4.

	//  r_x  r_y  r_z   0
	//  u_x  u_y  u_z   0
	// -f_x -f_y -f_z   0
	//    0    0    0   1

	mat4 tmp_view_matrix = mat4(mat3(view_matrix)); 

	gl_Position = projection_matrix * tmp_view_matrix * vec4(in_position, 1.0);
	tex_coord = in_position;
}