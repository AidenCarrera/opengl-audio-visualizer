#version 410 core

in VS_OUT {
    vec3 N;
    vec3 P;
} fs_in;

out vec4 FragColor;

// Environment map
uniform samplerCube ref_tex;

// Uniforms assigned from the C++ application
uniform vec3 camera_pos;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform float ambient_intensity;
uniform float shininess;

void main()
{
    // Setup Base Vectors
    vec3 N = normalize(fs_in.N); // Normal
    vec3 V = normalize(camera_pos - fs_in.P); // View direction

    // Environment Mapping
    // Reflect view vector around the normal to find where we're looking in the cubemap
    vec3 R_env = reflect(-V, N);
    vec3 env_color = texture(ref_tex, R_env).rgb;

    // Phong Reflection
    vec3 L = normalize(light_pos - fs_in.P); // Light direction
    vec3 R_light = reflect(-L, N);           // Reflected light direction

    // Ambient
    vec3 ambient = ambient_intensity * light_color;

    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * light_color;

    // Specular
    float spec = pow(max(dot(V, R_light), 0.0), shininess);
    vec3 specular = spec * light_color;

    // Combine Environment Map with Phong Lighting
    vec3 final_color = (ambient + diffuse) * env_color + specular;

    FragColor = vec4(final_color, 1.0);
}