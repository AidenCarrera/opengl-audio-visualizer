#pragma once
#include <iostream>
#include <cmath>
#include <cstring>

// Suppress warnings from third-party header
#pragma warning(push, 0)
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#pragma warning(pop)

// Include KISS FFT for Real-to-Complex Fast Fourier Transforms
extern "C" {
#include "kiss_fftr.h"
}

#include "vermilion.h" // For glfwGetTime

#ifndef M_PI_F
#define M_PI_F 3.14159265358979f
#endif

static const int FFT_SIZE = 1024;

// Audio system for loading and analyzing audio files
struct AudioSystem {
    // Audio file and engine data
    float* samples;
    ma_uint64 total_frames;
    ma_uint32 sample_rate;
    bool loaded;

    ma_engine engine;
    bool engine_ok;
    double start_time;
    bool playing;
    char file_path[256];

    // Playback state
    bool is_paused;
    double pause_accum;
    double pause_start_time;

    // Visualizer output data
    float amplitude, bass, mid, treble;

    // Internal DSP state
    float pk_amp, pk_bass, pk_mid, pk_treble; // Peak tracking for auto-scaling
    kiss_fftr_cfg fft_cfg;
    kiss_fft_scalar fft_in[FFT_SIZE];
    kiss_fft_cpx fft_out[FFT_SIZE / 2 + 1];

    AudioSystem()
        : samples(0), total_frames(0), sample_rate(44100),
        loaded(false), engine_ok(false), start_time(0), playing(false),
        is_paused(false), pause_accum(0), pause_start_time(0),
        amplitude(0), bass(0), mid(0), treble(0),
        pk_amp(0.001f), pk_bass(0.001f), pk_mid(0.001f), pk_treble(0.001f) {

        file_path[0] = '\0';
        fft_cfg = kiss_fftr_alloc(FFT_SIZE, 0, NULL, NULL);
    }

    bool load(const char* path) {
        ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, 1, 0);
        ma_decoder dec;
        if (ma_decoder_init_file(path, &cfg, &dec) != MA_SUCCESS) return false;

        sample_rate = dec.outputSampleRate;
        ma_uint64 len = 0;
        ma_decoder_get_length_in_pcm_frames(&dec, &len);
        if (len == 0) { ma_decoder_uninit(&dec); return false; }

        samples = new float[(size_t)len];
        ma_uint64 read = 0;
        ma_decoder_read_pcm_frames(&dec, samples, len, &read);
        total_frames = read;
        ma_decoder_uninit(&dec);

        loaded = true;
        strncpy(file_path, path, sizeof(file_path) - 1);
        file_path[sizeof(file_path) - 1] = '\0';
        std::cout << "[Audio] Loaded: " << path
            << " (" << total_frames << " frames, "
            << sample_rate << " Hz, "
            << (total_frames / (float)sample_rate) << "s)" << std::endl;
        return true;
    }

    bool try_load() {
        const char* potential_paths[] = {
            "../assets/audio.wav", "../assets/audio.mp3", "../assets/audio.flac",
            "../assets/music.wav", "../assets/music.mp3", "../assets/song.wav",
            "../assets/song.mp3", NULL
        };
        bool found = false;
        for (int i = 0; potential_paths[i]; i++) {
            if (load(potential_paths[i])) {
                found = true;
                break;
            }
        }
        if (!found) {
            std::cout << "[Audio] No audio file found. Place audio.wav or audio.mp3 in assets/" << std::endl;
            std::cout << "[Audio] Using time-based animation fallback" << std::endl;
            return false;
        }
        return true;
    }

    void play() {
        if (!loaded) return;
        if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
            std::cout << "[Audio] Engine init failed" << std::endl;
            return;
        }
        engine_ok = true;
        if (ma_engine_play_sound(&engine, file_path, NULL) == MA_SUCCESS) {
            start_time = glfwGetTime();
            playing = true;
            std::cout << "[Audio] Playing..." << std::endl;
        }
        else {
            std::cout << "[Audio] Failed to play sound" << std::endl;
        }
    }

    void toggle_pause() {
        if (!playing) return;
        is_paused = !is_paused;
        if (is_paused) {
            pause_start_time = glfwGetTime();
            ma_engine_stop(&engine);
            std::cout << "[Audio] Paused" << std::endl;
        }
        else {
            pause_accum += glfwGetTime() - pause_start_time;
            ma_engine_start(&engine);
            std::cout << "[Audio] Resumed" << std::endl;
        }
    }

    void analyze(double now) {
        if (!loaded) return;

        // Apply decay if paused so visuals smooth out
        if (is_paused) {
            amplitude *= 0.95f; bass *= 0.95f;
            mid *= 0.95f; treble *= 0.95f;
            return;
        }

        // Calculate current audio playback position
        double t = playing ? (now - start_time - pause_accum) : 0.0;
        if (t < 0) t = 0;

        ma_uint64 center = (ma_uint64)(t * sample_rate);
        if (center + FFT_SIZE >= total_frames) {
            amplitude *= 0.95f; bass *= 0.95f;
            mid *= 0.95f; treble *= 0.95f;
            return;
        }

        // Step 1: Windowing (Smooth the audio chunk edges)
        float rms = 0;
        for (int i = 0; i < FFT_SIZE; i++) {
            float sample = samples[center + i];
            rms += sample * sample;

            // Hann window function
            float w = 0.5f * (1.0f - cosf(2.0f * M_PI_F * i / (FFT_SIZE - 1)));
            fft_in[i] = sample * w;
        }
        float raw_amp = sqrtf(rms / FFT_SIZE);

        // Step 2: Fast Fourier Transform
        kiss_fftr(fft_cfg, fft_in, fft_out);

        // Step 3: Frequency binning (Isolate bass, mid, treble)
        float raw_bass = 0, raw_mid = 0, raw_treble = 0;
        float bin_hz = (float)sample_rate / FFT_SIZE;

        for (int k = 1; k < FFT_SIZE / 2; k++) {
            float mag = sqrtf(fft_out[k].r * fft_out[k].r + fft_out[k].i * fft_out[k].i) / FFT_SIZE;
            float freq = k * bin_hz;

            if (freq < 300.0f)       raw_bass += mag;   // Sub & Bass
            else if (freq < 2000.0f) raw_mid += mag;    // Mids & Vocals
            else                     raw_treble += mag; // High hats & Cymbals
        }

        // Step 4: Auto-scaling (Ensure it works for quiet and loud songs)
        // Track the highest value seen so far
        if (raw_amp > pk_amp)       pk_amp = raw_amp;
        if (raw_bass > pk_bass)     pk_bass = raw_bass;
        if (raw_mid > pk_mid)       pk_mid = raw_mid;
        if (raw_treble > pk_treble) pk_treble = raw_treble;

        // Normalize current values against the peak (0.0 to 1.0 range)
        raw_amp /= pk_amp;
        raw_bass /= pk_bass;
        raw_mid /= pk_mid;
        raw_treble /= pk_treble;

        // Step 5: Envelope follower (Smooth the visual bouncing)
        // Fast attack (reacts quickly to beats)
        // Slow decay (fades out smoothly)
        const float ATK = 0.4f, DCY = 0.90f;
        amplitude = raw_amp > amplitude ? amplitude * (1 - ATK) + raw_amp * ATK : amplitude * DCY;
        bass = raw_bass > bass ? bass * (1 - ATK) + raw_bass * ATK : bass * DCY;
        mid = raw_mid > mid ? mid * (1 - ATK) + raw_mid * ATK : mid * DCY;
        treble = raw_treble > treble ? treble * (1 - ATK) + raw_treble * ATK : treble * DCY;
    }

    void cleanup() {
        if (engine_ok) ma_engine_uninit(&engine);
        delete[] samples;
        samples = 0;
        loaded = false;

        kiss_fft_free(fft_cfg);
    }
};