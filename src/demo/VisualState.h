#pragma once
#include <cmath>

#ifndef M_PI_F
#define M_PI_F 3.14159265358979f
#endif

static const int NUM_OBJECTS = 47;

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

    void updateTime(float t) {
        time = t;
        time_drive = 0.5f * (1.0f + sinf(t));
    }

    void updateAudio(float a, float b, float m, float t) {
        amplitude = a; bass = b; mid = m; treble = t;
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

        // 2. Base Object Colors
        float* sphere_hues = white_spheres;
        float* torus_hues = white_torus;

        // Object animation pipeline

        // Layer 0: Central object (Bunny)
        // Hovers slightly in the middle and rotates constantly
        obj[0].pos[0] = sinf(rt * 0.7f) * 0.02f;
        obj[0].pos[1] = sinf(rt * 1.2f) * 0.015f;
        obj[0].pos[2] = cosf(rt * 0.5f) * 0.02f;
        obj[0].rot_y = rt * 30.0f;
        obj[0].scale = 1.0f;

        obj[0].tint[0] = 2.0f; obj[0].tint[1] = 2.0f; obj[0].tint[2] = 2.0f;
        obj[0].shine = 128.0f;


        // Layer 1: Inner ring (Bass)
        // 4 Objects orbiting tightly that react to bass sounds
        float ir = 0.50f; // Inner radius
        for (int i = 0; i < 4; i++) {
            int id = 1 + i;

            // Calculate orbital position
            float phase = i * 90.0f + rt * (35.0f + i * 5.0f);
            float ang = phase * M_PI_F / 180.0f;

            obj[id].pos[0] = cosf(ang) * ir;
            obj[id].pos[1] = sinf(rt * 3.5f + i * 1.57f) * 0.025f; // Vertical bobbing
            obj[id].pos[2] = sinf(ang) * ir;
            obj[id].rot_y = rt * 90.0f + i * 90.0f;

            // Apply Bass reactivity
            obj[id].scale = 0.10f + (dB * 0.25f);
            float pulse = 1.0f + (dB * 1.5f);

            obj[id].tint[0] = sphere_hues[i * 3 + 0] * pulse;
            obj[id].tint[1] = sphere_hues[i * 3 + 1] * pulse;
            obj[id].tint[2] = sphere_hues[i * 3 + 2] * pulse;
            obj[id].shine = 24.0f + (dB * 64.0f);
        }


        // Layer 2: Outer ring (Mids)
        // 4 Objects orbiting further out that react to vocals/melodies
        float or_ = 0.90f; // Outer radius
        for (int i = 0; i < 4; i++) {
            int id = 5 + i;

            float phase = i * 90.0f + 45.0f + rt * (18.0f + i * 3.0f);
            float ang = phase * M_PI_F / 180.0f;

            obj[id].pos[0] = cosf(ang) * or_;
            obj[id].pos[1] = cosf(rt * 2.8f + i * 1.8f) * 0.02f;
            obj[id].pos[2] = sinf(ang) * or_;
            obj[id].rot_y = rt * 140.0f + i * 45.0f;

            // Apply Mid reactivity
            obj[id].scale = 0.07f + (dM * 0.18f);
            float shimmer = 1.0f + (dM * 2.0f);

            obj[id].tint[0] = torus_hues[i * 3 + 0] * shimmer;
            obj[id].tint[1] = torus_hues[i * 3 + 1] * shimmer;
            obj[id].tint[2] = torus_hues[i * 3 + 2] * shimmer;
            obj[id].shine = 96.0f + (dM * 256.0f);
        }


        // Layer 3: Outermost ring (Treble)
        // 8 Objects orbiting furthest out that react to hi-hats/snares
        float outer_r = 1.30f;
        for (int i = 0; i < 8; i++) {
            int id = 9 + i;

            float phase = i * 45.0f - rt * (10.0f + i); // Negative rotation
            float ang = phase * M_PI_F / 180.0f;

            obj[id].pos[0] = cosf(ang) * outer_r;
            obj[id].pos[1] = sinf(rt * 1.5f + i * 0.8f) * 0.06f;
            obj[id].pos[2] = sinf(ang) * outer_r;
            obj[id].rot_y = rt * 100.0f + i * 45.0f;

            // Apply Treble reactivity
            obj[id].scale = 0.04f + (dT * 0.10f);
            float flash = 1.0f + (dT * 0.8f);

            int hIdx = (i % 4) * 3; // Cycle through the 4 colors for 8 objects
            obj[id].tint[0] = torus_hues[hIdx + 0] * flash;
            obj[id].tint[1] = torus_hues[hIdx + 1] * flash;
            obj[id].tint[2] = torus_hues[hIdx + 2] * flash;
            obj[id].shine = 64.0f + (dT * 32.0f);
        }


        // Layer 4: Background particles (Reactive to overall amplitude)
        // 30 tiny objects floating in the background that react to overall sound
        for (int i = 0; i < 30; i++) {
            int id = 17 + i;

            float p_radius = 0.4f + (i % 10) * 0.25f; // Scattered depths
            float phase = i * 45.0f + rt * (10.0f - (i % 3) * 2.0f);
            float ang = phase * M_PI_F / 180.0f;
            float h_offset = sinf(i * 4.0f) * 0.7f;

            obj[id].pos[0] = cosf(ang) * p_radius;
            obj[id].pos[1] = h_offset + sinf(rt * 0.5f + i) * 0.02f;
            obj[id].pos[2] = sinf(ang) * p_radius;
            obj[id].rot_y = rt * 20.0f + i * 33.0f;

            // Apply Amplitude reactivity
            obj[id].scale = 0.005f + (dA * 0.010f);

            obj[id].tint[0] = 3.0f; obj[id].tint[1] = 3.0f; obj[id].tint[2] = 3.0f;
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