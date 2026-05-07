#pragma once
#include <cmath>

#ifndef M_PI_F
#define M_PI_F 3.14159265358979f
#endif

static const int NUM_OBJECTS = 61;

// Color themes
static float white_spheres[12] = { 3.0f, 0.4f, 1.5f,  2.5f, 1.8f, 0.2f,  0.3f, 3.0f, 0.5f,  1.2f, 0.3f, 3.0f };
static float white_torus[12] = { 0.3f, 2.5f, 3.0f,  3.0f, 0.3f, 2.5f,  1.0f, 3.0f, 0.2f,  3.0f, 1.5f, 0.2f };

// Visual state and per-object transformations
struct ObjectState {
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
    ObjectState obj[NUM_OBJECTS];
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

        // Object animation pipeline

        // Layer 0: Central object (Bunny)
        // Hovers slightly in the middle and rotates constantly
        obj[0].pos[0] = sinf(rt * 0.7f) * 0.02f;
        obj[0].pos[1] = sinf(rt * 1.2f) * 0.015f;
        obj[0].pos[2] = cosf(rt * 0.5f) * 0.02f;
        obj[0].rot_y = rt * 30.0f;
        obj[0].scale = 1.0f;

        if (use_dynamic_colors) {
            float r0, g0, b0;
            hsv2rgb(rt * 20.0f + dA * 60.0f, 0.7f, 2.0f + dA * 0.5f, r0, g0, b0);
            obj[0].tint[0] = r0; obj[0].tint[1] = g0; obj[0].tint[2] = b0;
        } else {
            obj[0].tint[0] = 2.0f; obj[0].tint[1] = 2.0f; obj[0].tint[2] = 2.0f;
        }
        obj[0].shine = 128.0f;


        // Layer 1: Inner ring (Bass)
        // 4 Objects orbiting tightly that react to bass sounds
        float ir = radius_bass;
        for (int i = 0; i < 4; i++) {
            int id = 1 + i;

            // Calculate orbital position with uniform speed to prevent clipping
            float phase = i * 90.0f + rt * 45.0f;
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
                obj[id].tint[0] = white_spheres[i * 3 + 0] * pulse;
                obj[id].tint[1] = white_spheres[i * 3 + 1] * pulse;
                obj[id].tint[2] = white_spheres[i * 3 + 2] * pulse;
            }

            obj[id].shine = 24.0f + (dB * 64.0f);
        }


        // Layer 2: Outer ring (Mids)
        // 8 Objects orbiting further out that react to vocals/melodies
        float or_ = radius_mid;
        for (int i = 0; i < 8; i++) {
            int id = 5 + i;

            float phase = i * 45.0f + 45.0f + rt * 30.0f; // Adjust to 45 degree spacing for 8 objects
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
        }


        // Layer 3: Outermost ring (Treble)
        // 18 Objects orbiting furthest out that react to hi-hats/snares
        float outer_r = radius_treble;
        for (int i = 0; i < 18; i++) {
            int id = 13 + i; // Start at 13 because layer 2 goes up to 12

            float phase, current_outer, h_offset;
            if (i < 8) {
                // First 8 cubes start grouped in a line (20 degrees apart).
                // They orbit together but have a tiny speed difference (i * 0.5f) so they slowly stretch into a long string!
                phase = i * 12.0f - rt * (20.0f + i * 0.5f); 
                current_outer = outer_r; // Clean uniform radius
                h_offset = (i - 3.5f) * 0.05f; // Slight geometric height slope
            } else {
                // Remaining 10 cubes are sporadic and wildly orbit the scene
                phase = i * 137.0f - rt * (10.0f + i * 5.0f);
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
        }


        // Layer 4: Background particles (Reactive to overall amplitude)
        // 30 tiny objects floating in the background that react to overall sound
        for (int i = 0; i < 30; i++) {
            int id = 31 + i; // Starts at 31 because layer 3 goes up to 30

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