#pragma once
#include <cmath>

#ifndef M_PI_F
#define M_PI_F 3.14159265358979f
#endif

static const int MAX_OBJECTS = 256;

// Color themes
static float white_spheres[12] = { 3.0f, 0.4f, 1.5f,  2.5f, 1.8f, 0.2f,  0.3f, 3.0f, 0.5f,  1.2f, 0.3f, 3.0f };
static float white_torus[12] = { 0.3f, 2.5f, 3.0f,  3.0f, 0.3f, 2.5f,  1.0f, 3.0f, 0.2f,  3.0f, 1.5f, 0.2f };

// Visual state and per-object transformations
struct ObjectState {
    bool active;
    int mesh_id;
    float pos[3];
    float rot_y;
    float scale;
    float tint[3];    // per-object light color
    float shine;      // per-object shininess
};

struct VisualState {
    // Inputs
    float time;
    float time_drive;
    float amplitude, bass, mid, treble;
    bool audio_active;

    // Outputs
    ObjectState obj[MAX_OBJECTS];
    float light_pos[3];

    // User controls
    float reaction_multiplier = 1.0f;
    float rot_time = 0.0f;
    bool use_dynamic_colors = false;

    // Radius controls
    float radius_bass = 0.50f;
    float radius_mid = 1.10f;
    float radius_treble = 1.50f;
    float radius_particles = 0.50f;

    // Object counts
    int num_bass = 4;
    int num_mid = 8;
    int num_treble = 18;
    int num_particles = 30;

    void updateTime(float t) {
        time = t;
        time_drive = 0.5f * (1.0f + sinf(t));
    }

    void updateAudio(float a, float b, float m, float t) {
        amplitude = a; bass = b; mid = m; treble = t;
    }

    // Helper to convert Hue, Saturation, Value to RGB
    void hsv2rgb(float h, float s, float v, float& r, float& g, float& b) {
        float c = v * s;
        float h_prime = std::fmod(h, 360.0f) / 60.0f;
        if (h_prime < 0.0f) h_prime += 6.0f;
        float x = c * (1.0f - std::abs(std::fmod(h_prime, 2.0f) - 1.0f));
        float m = v - c;

        if (h_prime < 1.0f) { r = c; g = x; b = 0; }
        else if (h_prime < 2.0f) { r = x; g = c; b = 0; }
        else if (h_prime < 3.0f) { r = 0; g = c; b = x; }
        else if (h_prime < 4.0f) { r = 0; g = x; b = c; }
        else if (h_prime < 5.0f) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }

        r += m; g += m; b += m;
    }

    void computeVisuals(float base_theta, bool has_audio) {
        audio_active = has_audio;
        float t = time;
        float rt = rot_time;

        // 1. Determine Animation Drivers (Use audio if playing, otherwise auto-pulse over time)
        float dA, dB, dM, dT;
        if (has_audio) {
            dA = amplitude * reaction_multiplier;
            dB = bass * reaction_multiplier;
            dM = mid * reaction_multiplier;
            dT = treble * reaction_multiplier;
        }
        else {
            dA = time_drive * reaction_multiplier;
            dB = 0.5f * (1.0f + sinf(t * 1.5f)) * 0.5f * reaction_multiplier;
            dM = 0.5f * (1.0f + sinf(t * 2.3f)) * 0.5f * reaction_multiplier;
            dT = 0.5f * (1.0f + sinf(t * 3.7f)) * 0.5f * reaction_multiplier;
        }

        // Clear all objects
        for (int i = 0; i < MAX_OBJECTS; i++) {
            obj[i].active = false;
        }

        int id = 0;

        // Layer 0: Central object (Bunny)
        // Hovers slightly in the middle and rotates constantly
        obj[id].active = true;
        obj[id].mesh_id = 0; // MESH_BUNNY
        obj[id].pos[0] = sinf(rt * 0.7f) * 0.02f;
        obj[id].pos[1] = sinf(rt * 1.2f) * 0.015f;
        obj[id].pos[2] = cosf(rt * 0.5f) * 0.02f;
        obj[id].rot_y = rt * 30.0f;
        obj[id].scale = 1.0f;

        if (use_dynamic_colors) {
            float r0, g0, b0;
            hsv2rgb(rt * 20.0f + dA * 60.0f, 0.7f, 2.0f + dA * 0.5f, r0, g0, b0);
            obj[id].tint[0] = r0; obj[id].tint[1] = g0; obj[id].tint[2] = b0;
        } else {
            obj[id].tint[0] = 2.0f; obj[id].tint[1] = 2.0f; obj[id].tint[2] = 2.0f;
        }
        obj[id].shine = 128.0f;
        id++;


        // Layer 1: Inner ring (Bass)
        // Objects orbiting tightly that react to bass sounds
        float ir = radius_bass;
        for (int i = 0; i < num_bass; i++) {
            if (id >= MAX_OBJECTS) break;
            
            obj[id].active = true;
            obj[id].mesh_id = 1; // MESH_SPHERE

            // Calculate orbital position with uniform speed to prevent clipping
            float phase = i * (360.0f / (num_bass > 0 ? num_bass : 1)) + rt * 45.0f;
            float ang = phase * M_PI_F / 180.0f;

            // Add static randomness to radius
            float r_offset = sinf(i * 123.45f) * 0.05f;
            float current_ir = ir + r_offset;

            obj[id].pos[0] = cosf(ang) * current_ir;
            obj[id].pos[1] = 0.0f; // Fixed flat level height
            obj[id].pos[2] = sinf(ang) * current_ir;
            obj[id].rot_y = rt * 90.0f + i * 90.0f;

            // Apply Bass reactivity
            obj[id].scale = 0.10f + (dB * 0.18f); // Reduced reactivity
            float pulse = 1.0f + (dB * 1.0f);     // Reduced reactivity

            if (use_dynamic_colors) {
                float hue = rt * 30.0f + i * 90.0f + dB * 80.0f;
                float r, g, b;
                hsv2rgb(hue, 1.0f, pulse * 1.5f, r, g, b);
                obj[id].tint[0] = r;
                obj[id].tint[1] = g;
                obj[id].tint[2] = b;
            } else {
                obj[id].tint[0] = white_spheres[(i % 4) * 3 + 0] * pulse;
                obj[id].tint[1] = white_spheres[(i % 4) * 3 + 1] * pulse;
                obj[id].tint[2] = white_spheres[(i % 4) * 3 + 2] * pulse;
            }

            obj[id].shine = 24.0f + (dB * 64.0f);
            id++;
        }


        // Layer 2: Outer ring (Mids)
        // Objects orbiting further out that react to vocals/melodies
        float or_ = radius_mid;
        for (int i = 0; i < num_mid; i++) {
            if (id >= MAX_OBJECTS) break;
            
            obj[id].active = true;
            obj[id].mesh_id = 2; // MESH_TORUS

            float phase = i * (360.0f / (num_mid > 0 ? num_mid : 1)) + 45.0f + rt * 30.0f;
            float ang = phase * M_PI_F / 180.0f;

            // Add static randomness to radius and height
            float r_offset = sinf(i * 321.12f) * 0.08f;
            float h_offset = sinf(i * 666.66f) * 0.35f; // Greatly increased vertical displacement
            float current_or = or_ + r_offset;

            obj[id].pos[0] = cosf(ang) * current_or;
            obj[id].pos[1] = h_offset + cosf(rt * 2.8f + i * 1.8f) * 0.1f; // Increased vertical bobbing
            obj[id].pos[2] = sinf(ang) * current_or;
            obj[id].rot_y = rt * 140.0f + i * 45.0f;

            // Apply Mid reactivity
            obj[id].scale = 0.07f + (dM * 0.12f); // Reduced reactivity
            float shimmer = 1.0f + (dM * 1.3f);   // Reduced reactivity

            if (use_dynamic_colors) {
                float hue = 360.0f - (rt * 45.0f) + i * 90.0f + dM * 100.0f;
                float r, g, b;
                hsv2rgb(hue, 0.8f + dM * 0.1f, shimmer * 1.5f, r, g, b);
                obj[id].tint[0] = r;
                obj[id].tint[1] = g;
                obj[id].tint[2] = b;
            } else {
                int hIdx = (i % 4) * 3;
                obj[id].tint[0] = white_torus[hIdx + 0] * shimmer;
                obj[id].tint[1] = white_torus[hIdx + 1] * shimmer;
                obj[id].tint[2] = white_torus[hIdx + 2] * shimmer;
            }

            obj[id].shine = 96.0f + (dM * 256.0f);
            id++;
        }


        // Layer 3: Outermost ring (Treble)
        // Objects orbiting furthest out that react to hi-hats/snares
        float outer_r = radius_treble;
        for (int i = 0; i < num_treble; i++) {
            if (id >= MAX_OBJECTS) break;
            
            obj[id].active = true;
            obj[id].mesh_id = 3; // MESH_CUBE

            float phase, current_outer, h_offset;
            if (i < num_treble / 2) {
                // First half of cubes start grouped in a line (20 degrees apart).
                // They orbit together but have a tiny speed difference (i * 0.1f) so they slowly stretch into a long string!
                phase = i * 12.0f - rt * (20.0f + i * 0.1f); 
                current_outer = outer_r; // Clean uniform radius
                h_offset = (i - (num_treble / 4.0f)) * 0.05f; // Slight geometric height slope
            } else {
                // Remaining cubes are sporadic and wildly orbit the scene
                // Bounded pseudo-random speed so it doesn't spin wildly out of control at high object counts
                phase = i * 137.0f - rt * (25.0f + sinf(i * 321.0f) * 15.0f);
                current_outer = outer_r + sinf(i * 456.78f) * 0.35f; // Sporadic radius
                h_offset = sinf(i * 777.77f) * 0.3f; // Sporadic height
            }
            float ang = phase * M_PI_F / 180.0f;

            obj[id].pos[0] = cosf(ang) * current_outer;
            obj[id].pos[1] = h_offset + sinf(rt * 1.5f + i * 0.8f) * 0.08f;
            obj[id].pos[2] = sinf(ang) * current_outer;
            obj[id].rot_y = rt * 100.0f + i * 45.0f;

            // Apply Treble reactivity
            obj[id].scale = 0.04f + (dT * 0.10f);
            float flash = 1.0f + (dT * 0.8f);

            if (use_dynamic_colors) {
                float hue = rt * 120.0f + i * 45.0f + dT * 130.0f;
                float r, g, b;
                hsv2rgb(hue, 0.6f + dT * 0.2f, flash * 1.5f, r, g, b);
                obj[id].tint[0] = r;
                obj[id].tint[1] = g;
                obj[id].tint[2] = b;
            } else {
                int hIdx = (i % 4) * 3;
                obj[id].tint[0] = white_torus[hIdx + 0] * flash;
                obj[id].tint[1] = white_torus[hIdx + 1] * flash;
                obj[id].tint[2] = white_torus[hIdx + 2] * flash;
            }

            obj[id].shine = 64.0f + (dT * 32.0f);
            id++;
        }


        // Layer 4: Background particles (Reactive to overall amplitude)
        // tiny objects floating in the background that react to overall sound
        for (int i = 0; i < num_particles; i++) {
            if (id >= MAX_OBJECTS) break;
            
            obj[id].active = true;
            obj[id].mesh_id = 1; // MESH_SPHERE

            float p_radius = radius_particles + (i % 10) * 0.3f + sinf(i * 789.12f) * 0.1f;
            float phase = i * 45.0f + rt * 15.0f; // Uniform speed
            float ang = phase * M_PI_F / 180.0f;
            float h_offset = sinf(i * 4.0f) * 0.7f;

            obj[id].pos[0] = cosf(ang) * p_radius;
            obj[id].pos[1] = h_offset + sinf(rt * 0.5f + i) * 0.02f;
            obj[id].pos[2] = sinf(ang) * p_radius;
            obj[id].rot_y = rt * 20.0f + i * 33.0f;

            // Apply Amplitude reactivity
            obj[id].scale = 0.005f + (dA * 0.010f);

            if (use_dynamic_colors) {
                float hue = rt * 15.0f + i * 12.0f + dA * 40.0f;
                float r, g, b;
                hsv2rgb(hue, 0.5f, 1.5f + dA * 0.5f, r, g, b);
                obj[id].tint[0] = r; obj[id].tint[1] = g; obj[id].tint[2] = b;
            } else {
                obj[id].tint[0] = 3.0f; obj[id].tint[1] = 3.0f; obj[id].tint[2] = 3.0f;
            }

            obj[id].shine = 32.0f;
            id++;
        }

        // Dynamic global lighting
        // Light orbits the scene and pushes outward on loud parts
        float la = (base_theta + rt * 50.0f + dB * 150.0f) * M_PI_F / 180.0f;
        float lr = 1.5f + (dA * 1.5f);

        light_pos[0] = cosf(la) * lr;
        light_pos[1] = 0.8f + (dB * 2.0f) + (sinf(rt * 1.5f) * 0.3f);
        light_pos[2] = sinf(la) * lr;
    }
};