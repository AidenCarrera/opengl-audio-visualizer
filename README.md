# Audio Reactive 3D Visualizer

A real-time OpenGL 4.5 audio visualizer that reacts to music using FFT frequency analysis. Objects orbit, pulse, and shift color in response to bass, mids, and treble extracted live from any audio file.

Built with C++, GLFW, OpenGL, ImGui, KissFFT, and miniaudio.

![OpenGL 4.5](https://img.shields.io/badge/OpenGL-4.5-blue) ![Platform](https://img.shields.io/badge/platform-Windows-lightgrey) ![Build](https://img.shields.io/badge/build-CMake-orange)

---

## Tech Stack
- Language/Graphics: C++17, OpenGL 4.5
- Windowing/UI: GLFW, ImGui
- Audio: miniaudio (playback), KissFFT (analysis)
- Build System: CMake

---

## Features

- **Real-time FFT audio analysis** — Hann-windowed 1024-point FFT with auto-scaling peak normalization and envelope following
- **Frequency-reactive layers** — Bass, mid, and treble bands each drive a separate ring of objects (spheres, toruses, cubes)
- **Environment mapping** — Phong shading with cubemap reflection on all scene objects
- **Cubemap skybox** — Full 360° environment rendered behind the scene
- **Dynamic audio colors** — Optional HSV color mode that shifts hue in response to amplitude and frequency
- **ImGui control panel** — Adjust object counts, ring radii, reaction intensity, lighting, and camera in real time
- **Hotswap audio** — Load any `.wav`, `.mp3`, or `.flac` file at runtime via Windows file dialog
- **Fallback animation** — Time-based sine wave animation plays when no audio file is loaded

---

## Controls

### Keyboard

| Key | Action |
|---|---|
| `Space` | Play / Pause |
| `1` / `2` / `3` / `4` / `5` | Light color: White / Red / Green / Blue / Dynamic audio color|
| `A` / `D` | Orbit camera left / right |
| `W` / `S` | Zoom in / out |
| `↑` / `↓` | Camera height |
| `←` / `→` | Rotate light |
| `+` / `-` | Ambient intensity |
| `Q` / `E` | Reaction multiplier |

### Mouse

| Input | Action |
|---|---|
| Left drag | Orbit camera |
| Scroll wheel | Zoom |

---

## Building

Requires CMake and Visual Studio (MSVC). GLFW is built from source — no pre-built binaries needed.

```sh
mkdir build
cd build
cmake -G "Visual Studio 18" ..
```

Then open `build/vermilion9.sln` in Visual Studio and build the `demo` target. The executable is placed in `bin/`.

---

## Adding Audio

The project includes a default demo file in the /assets directory.
To use your own music, place a file in the /assets folder using one of the following filenames for automatic loading on launch:

```
assets/audio.wav
assets/audio.mp3
assets/audio.flac
assets/music.wav
assets/music.mp3
assets/song.wav
assets/song.mp3
```

Or use the **Load Audio File...** button in the ImGui panel to load any file at runtime.

---

## Dependencies

All dependencies are vendored in the repository. No external package managers required.

| Library | Purpose | Location |
|---|---|---|
| [GLFW](https://github.com/glfw/glfw) | Window & input | `lib/glfw/` |
| [gl3w](https://github.com/skaslev/gl3w) | OpenGL loader | `lib/gl3w.c` |
| [Dear ImGui](https://github.com/ocornut/imgui) | Debug UI | `external/imgui/` |
| [KissFFT](https://github.com/mborgerding/kissfft) | FFT analysis | `external/kissfft/` |
| [miniaudio](https://miniaud.io/) | Audio playback | `include/miniaudio.h` |
| [stb_image](https://github.com/nothings/stb) | Texture loading | `include/stb_image.h` |

---

## Project Structure

```
assets/         Meshes, cubemap textures, shaders, and audio files
external/       Third-party libraries (ImGui, KissFFT)
include/        Project headers and single-file libraries
lib/            Vermilion utility library source + GLFW source
src/demo/       Main application source
tools/          Standalone VBM mesh conversion utilities
```
