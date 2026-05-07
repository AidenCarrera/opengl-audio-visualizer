// Computer Graphics CS4143/5143
// Final Project: Audio Reactive 3D Visualizer (Phong + Environment Mapping)

#include "vermilion.h"

#include "vapp.h"
#include "vutils.h"
#include "vmath.h"
#include "vbm.h"

#include "vgl.h"
#include "LoadShaders.h"
#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "AudioSystem.h"
#include "VisualState.h"

using namespace std;

static const int MESH_BUNNY = 0;
static const int MESH_SPHERE = 1;
static const int MESH_TORUS = 2;
static const int MESH_CUBE = 3;
static const int NUM_MESHES = 4;

// OpenGL state variables

enum VAO_IDs { Axis, Cube, CubeElement, NumVAOs };
enum Buffer_IDs { ArrayBufferAxis, ArrayBufferCube, ArrayBufferCubeElement, NumBuffers };
enum Attrib_IDs { vColor = 0, vPosition = 1 };
enum Cube_Attrib_IDs { cPosition = 0 };
enum Texture_IDs { TextureCube, NumTextures };

GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint Textures[NumTextures];

GLuint program, axis_program, skybox_program;

GLuint render_model_matrix_loc, render_view_matrix_loc, render_projection_matrix_loc;
GLuint ambient_intensity_loc, shininess_loc, light_pos_loc, light_color_loc, camera_pos_loc;
GLuint axis_model_matrix_loc, axis_view_matrix_loc, axis_projection_matrix_loc;
GLuint skybox_view_loc, skybox_projection_loc;

vmath::mat4 model_matrix, view_matrix, projection_matrix;

// Meshes to render
VBObject meshes[NUM_MESHES];

// Map scene objects to their respective meshes
inline int getMeshIndex(int id) {
    if (id == 0) return MESH_BUNNY;
    if (id <= 4) return MESH_SPHERE;
    if (id <= 8) return MESH_TORUS;
    if (id <= 16) return MESH_CUBE;
    return MESH_SPHERE; // particles
}

float aspect;

// Camera setup
vmath::vec3 eye(0.0f, 0.25f, 3.0f);
vmath::vec3 center(0.0f, 0.0f, 0.0f);
vmath::vec3 up(0.0f, 1.0f, 0.0f);

// Interactive variables
float camera_orbit_angle = 0.0f;
float camera_height = 0.25f;
float camera_radius = 2.0f;
float reaction_multiplier = 1.0f;
bool auto_rotate = true;
float accumulated_rotation_time = 0.0f;
double last_time = 0.0f;

// Light setup
float ambient_intensity = 0.2f;
float light_theta = 90.0f;
vmath::vec3 light_color(1.0f, 1.0f, 1.0f);

int win_width = 1024;
int win_height = 768;

const GLuint NumVertices = 6;
static const float color_clear[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static const float depth_clear = 1.0f;

// Global instances
AudioSystem audio;
VisualState visual_state = {};

// Keyboard input handler

void Onkey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_1:
            light_color = vmath::vec3(1.0f, 1.0f, 1.0f);
            cout << "[1] Light Color: White" << endl;
            break;
        case GLFW_KEY_2:
            light_color = vmath::vec3(1.0f, 0.3f, 0.3f);
            cout << "[2] Light Color: Red" << endl;
            break;
        case GLFW_KEY_3:
            light_color = vmath::vec3(0.3f, 1.0f, 0.3f);
            cout << "[3] Light Color: Green" << endl;
            break;
        case GLFW_KEY_4:
            light_color = vmath::vec3(0.3f, 0.3f, 1.0f);
            cout << "[4] Light Color: Blue" << endl;
            break;
        case GLFW_KEY_SPACE:
            auto_rotate = !auto_rotate;
            audio.toggle_pause();
            cout << "[Space] Pause / Play" << endl;
            break;
        }
    }
}

void processInput(GLFWwindow* window, float dt)
{
    float orbit_speed = 90.0f; // degrees per sec
    float light_speed = 90.0f; // degrees per sec
    float zoom_speed = 2.0f; // units per sec
    float height_speed = 2.0f; // units per sec
    float intensity_speed = 1.0f; // per sec

    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        ambient_intensity += intensity_speed * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        ambient_intensity -= intensity_speed * dt;
        if (ambient_intensity < 0.0f) ambient_intensity = 0.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        light_theta += light_speed * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        light_theta -= light_speed * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera_orbit_angle -= orbit_speed * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera_orbit_angle += orbit_speed * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera_radius -= zoom_speed * dt;
        if (camera_radius < 0.1f) camera_radius = 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera_radius += zoom_speed * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        camera_height += height_speed * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        camera_height -= height_speed * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        reaction_multiplier -= intensity_speed * dt;
        if (reaction_multiplier < 0.0f) reaction_multiplier = 0.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        reaction_multiplier += intensity_speed * dt;
    }
}

void init(void)
{
    // Axis vertices
    struct VertexData { GLubyte color[4]; GLfloat position[4]; };
    VertexData axis_vertices[NumVertices] = {
        {{255, 0, 0, 255}, {0.0f, 0.0f, 0.0f, 1.0f}},
        {{255, 0, 0, 255}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0, 255, 0, 255}, {0.0f, 0.0f, 0.0f, 1.0f}},
        {{0, 255, 0, 255}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0, 0, 255, 255}, {0.0f, 0.0f, 0.0f, 1.0f}},
        {{0, 0, 255, 255}, {0.0f, 0.0f, 1.0f, 1.0f}}
    };

    glCreateVertexArrays(NumVAOs, VAOs);
    glBindVertexArray(VAOs[Axis]);
    glCreateBuffers(NumBuffers, Buffers);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBufferAxis]);
    glNamedBufferStorage(Buffers[ArrayBufferAxis], sizeof(axis_vertices), axis_vertices, 0);
    glVertexAttribPointer(vColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexData), BUFFER_OFFSET(0));
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(axis_vertices[0].color)));
    glEnableVertexAttribArray(vColor);
    glEnableVertexAttribArray(vPosition);
    glBindVertexArray(0);

    // Skybox cube
    GLfloat cube_verts[] = {
        -100,-100,-100, -100,-100,100, -100,100,-100, -100,100,100,
         100,-100,-100,  100,-100,100,  100,100,-100,  100,100,100
    };
    glBindVertexArray(VAOs[Cube]);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBufferCube]);
    glNamedBufferStorage(Buffers[ArrayBufferCube], sizeof(cube_verts), cube_verts, 0);
    glVertexAttribPointer(cPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(cPosition);
    glBindVertexArray(0);

    GLushort cube_idx[] = { 0,1,2,3,6,7,4,5, 2,6,0,4,1,5,3,7 };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[ArrayBufferCubeElement]);
    glNamedBufferStorage(Buffers[ArrayBufferCubeElement], sizeof(cube_idx), cube_idx, 0);

    // Load meshes
    meshes[MESH_BUNNY].LoadFromVBM("../assets/bunny.vbm", 0, 1, 2);
    meshes[MESH_SPHERE].LoadFromVBM("../assets/unit_sphere.vbm", 0, 1, 2);
    meshes[MESH_TORUS].LoadFromVBM("../assets/unit_torus.vbm", 0, 1, 2);
    meshes[MESH_CUBE].LoadFromVBM("../assets/unit_cube.vbm", 0, 1, 2);

    // Object shader (environment mapping + Phong)
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER,   "../assets/shaders/reflection_mesh/reflection_mesh.vert" },
        { GL_FRAGMENT_SHADER, "../assets/shaders/reflection_mesh/reflection_mesh.frag" },
        { GL_NONE, NULL }
    };
    program = LoadShaders(shaders);

    render_model_matrix_loc = glGetUniformLocation(program, "model_matrix");
    render_view_matrix_loc = glGetUniformLocation(program, "view_matrix");
    render_projection_matrix_loc = glGetUniformLocation(program, "projection_matrix");
    ambient_intensity_loc = glGetUniformLocation(program, "ambient_intensity");
    shininess_loc = glGetUniformLocation(program, "shininess");
    light_pos_loc = glGetUniformLocation(program, "light_pos");
    light_color_loc = glGetUniformLocation(program, "light_color");
    camera_pos_loc = glGetUniformLocation(program, "camera_pos");

    // Axis shader
    ShaderInfo axis_sh[] = {
        { GL_VERTEX_SHADER,   "../assets/shaders/axis/axis.vert" },
        { GL_FRAGMENT_SHADER, "../assets/shaders/axis/axis.frag" },
        { GL_NONE, NULL }
    };
    axis_program = LoadShaders(axis_sh);
    axis_model_matrix_loc = glGetUniformLocation(axis_program, "model_matrix");
    axis_view_matrix_loc = glGetUniformLocation(axis_program, "view_matrix");
    axis_projection_matrix_loc = glGetUniformLocation(axis_program, "projection_matrix");

    // Skybox shader
    ShaderInfo sky_sh[] = {
        { GL_VERTEX_SHADER,   "../assets/shaders/skybox/skybox.vert" },
        { GL_FRAGMENT_SHADER, "../assets/shaders/skybox/skybox.frag" },
        { GL_NONE, NULL }
    };
    skybox_program = LoadShaders(sky_sh);
    skybox_view_loc = glGetUniformLocation(skybox_program, "view_matrix");
    skybox_projection_loc = glGetUniformLocation(skybox_program, "projection_matrix");

    // Cubemap texture
    glGenTextures(1, &Textures[TextureCube]);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Textures[TextureCube]);

    std::vector<std::string> faces = {
        "../assets/px.png",
        "../assets/nx.png",
        "../assets/py.png",
        "../assets/ny.png",
        "../assets/pz.png",
        "../assets/nz.png"
    };

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "[ERROR] Cubemap texture failed to load at path: " << faces[i] << std::endl;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

// Rendering routine

void display(GLFWwindow* window)
{
    double now = glfwGetTime();
    float dt = (float)(now - last_time);
    last_time = now;
    
    processInput(window, dt);

    if (auto_rotate) {
        accumulated_rotation_time += dt;
    }

    // Update inputs
    visual_state.updateTime((float)now);
    visual_state.reaction_multiplier = reaction_multiplier;
    visual_state.rot_time = accumulated_rotation_time;

    audio.analyze(now);
    visual_state.updateAudio(audio.amplitude, audio.bass, audio.mid, audio.treble);

    // Derive all per-object visuals
    visual_state.computeVisuals(light_theta, audio.loaded);

    // GL setup
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glClearBufferfv(GL_COLOR, 0, color_clear);
    glClearBufferfv(GL_DEPTH, 0, &depth_clear);

    // Dynamic Camera calculating eye from orbit logic
    float rad = camera_orbit_angle * M_PI_F / 180.0f;
    eye[0] = camera_radius * sinf(rad);
    eye[1] = camera_height;
    eye[2] = camera_radius * cosf(rad);

    view_matrix = vmath::lookat(eye, center, up);
    projection_matrix = vmath::perspective(50.0f, 1.0f / aspect, 0.1f, 500.0f);

    // Render skybox
    glUseProgram(skybox_program);
    glBindTextureUnit(0, Textures[TextureCube]);
    glUniform1i(glGetUniformLocation(skybox_program, "tex"), 0);
    glUniformMatrix4fv(skybox_view_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(skybox_projection_loc, 1, GL_FALSE, projection_matrix);

    glBindVertexArray(VAOs[Cube]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Buffers[ArrayBufferCubeElement]);
    glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, NULL);
    glDrawElements(GL_TRIANGLE_STRIP, 8, GL_UNSIGNED_SHORT, BUFFER_OFFSET(8 * sizeof(GLushort)));

    // Render scene objects
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "ref_tex"), 0);
    glUniformMatrix4fv(render_view_matrix_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(render_projection_matrix_loc, 1, GL_FALSE, projection_matrix);
    glUniform1f(ambient_intensity_loc, ambient_intensity);
    glUniform3fv(light_pos_loc, 1, visual_state.light_pos);
    glUniform3fv(camera_pos_loc, 1, eye);

    for (int i = 0; i < NUM_OBJECTS; i++) {
        const ObjectState& o = visual_state.obj[i];

        // Per-object light color (base * tint)
        float lc[3] = {
            light_color[0] * o.tint[0],
            light_color[1] * o.tint[1],
            light_color[2] * o.tint[2]
        };
        glUniform3fv(light_color_loc, 1, lc);
        glUniform1f(shininess_loc, o.shine);

        model_matrix = vmath::translate(o.pos[0], o.pos[1], o.pos[2]) *
            vmath::rotate(0.0f, o.rot_y, 0.0f) *
            vmath::scale(o.scale);
        glUniformMatrix4fv(render_model_matrix_loc, 1, GL_FALSE, model_matrix);

        meshes[getMeshIndex(i)].Render(0, 1);
    }

    // Render axes for the central object
    glUseProgram(axis_program);
    model_matrix = vmath::translate(visual_state.obj[0].pos[0], visual_state.obj[0].pos[1], visual_state.obj[0].pos[2]) *
        vmath::rotate(0.0f, visual_state.obj[0].rot_y, 0.0f) *
        vmath::scale(visual_state.obj[0].scale * 0.3f);
    glUniformMatrix4fv(axis_model_matrix_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(axis_view_matrix_loc, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(axis_projection_matrix_loc, 1, GL_FALSE, projection_matrix);

    glLineWidth(2.0f);
    glBindVertexArray(VAOs[Axis]);
    glDrawArrays(GL_LINES, 0, 6);
    glLineWidth(1.0f);
    glBindVertexArray(0);
}

// Window resize callback and application entry point

void Resize(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
    aspect = float(height) / float(width);
}

int main(int argc, char** argv)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);

    cout << "Audio Visualizer - Aiden Carrera" << endl;

    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(win_width, win_height, "AIDEN CARRERA A20430776 - Audio Visualizer", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, Onkey);
    gl3wInit();

    init();

    // Load and play audio
    if (audio.try_load()) {
        audio.play();
    }

    cout << "--------------------------------------------" << endl;
    cout << "  Controls:" << endl;
    cout << "  Lighting:" << endl;
    cout << "    +/-        Ambient intensity" << endl;
    cout << "    Left/Right Light rotation" << endl;
    cout << "    1/2/3/4    Light color (W/R/G/B)" << endl;
    cout << "  Camera:" << endl;
    cout << "    A / D      Orbit left/right" << endl;
    cout << "    W / S      Zoom in/out" << endl;
    cout << "    Up/Down    Height adjust" << endl;
    cout << "  Audio:" << endl;
    cout << "    Q / E      Reaction intensity" << endl;
    cout << "    Space      Play / pause" << endl;

    cout << "--------------------------------------------" << endl;

    while (!glfwWindowShouldClose(window))
    {
        glfwGetWindowSize(window, &win_width, &win_height);
        Resize(0, 0, win_width, win_height);
        display(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    audio.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
