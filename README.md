# Squid Game: OpenGL Trials

<p align="center">
  <img src="docs/media/banner.jpg" alt="Gameplay showcase of the Squid Game OpenGL Trials arena" title="Squid Game: OpenGL Trials gameplay showcase" width="900">
</p>

<p align="center"><em>Figure 1 - Real-time OpenGL rendering with the doll, contestants, arena lighting, and environment models.</em></p>

## Feature tour

<p align="center">
  <video src="docs/media/feature-tour.mp4" poster="docs/media/banner.jpg" title="Squid Game: OpenGL Trials feature tour" controls preload="metadata" width="900">
    Your browser cannot play the embedded video. <a href="docs/media/feature-tour.mp4">Download the feature tour video</a>.
  </video>
</p>

<p align="center"><a href="docs/media/feature-tour.mp4">Download or open the full feature tour (MP4)</a></p>

An interactive, Squid Game-inspired 3D simulation built from scratch with C++ and modern OpenGL. Play **Red Light, Green Light** and **Glass Bridge**, explore the scenes through cinematic cameras, and inspect how each model is assembled from reusable geometric primitives.

> This is an independent educational project. It is not affiliated with, endorsed by, or sponsored by Netflix or the creators of *Squid Game*.

## Highlights

- Two playable rounds: Red Light, Green Light and Glass Bridge
- OpenGL 3.3 Core rendering with per-fragment Blinn-Phong lighting
- Directional, point, wall, and tracking spot lights
- Procedural models built from boxes, spheres, cylinders, cones, disks, and tori
- Textured sand, bark, and concrete environments
- 32 autonomous contestants, elimination animation, jumping, and glass-shard physics
- Multiple cinematic cameras plus a picture-in-picture doll camera
- Dear ImGui HUD and live controls
- Shape Inspector mode for learning and project demonstrations

## Tech stack

| Area | Technology |
| --- | --- |
| Language | C++17 and C |
| Graphics | OpenGL 3.3 Core, GLAD, GLFW, GLM |
| Interface | Dear ImGui |
| Audio | Windows MCI (`winmm`) |
| Build | CMake 3.20+ or MinGW-w64 `g++` |

## Quick start

### Requirements

- Windows 10 or later
- A GPU and driver with OpenGL 3.3 support
- CMake 3.20+
- A C++17 compiler
- GLFW and GLM development packages

With MSYS2 MinGW-w64, install the required toolchain:

```bash
pacman -S --needed mingw-w64-x86_64-toolchain \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-glfw \
  mingw-w64-x86_64-glm
```

Build from an MSYS2 MinGW 64-bit shell:

```bash
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Run the game from the repository root:

```powershell
.\build\bin\squid_game_opengl.exe
```

The build copies shaders and available runtime assets beside the executable, so it can also be launched directly from `build/bin`.

### Optional soundtrack

Music is not distributed with the repository. To use a locally owned MP3, place it at:

```text
assets/audio/squid_song.mp3
```

The file is intentionally ignored by Git. The simulation works without it.

## Controls

### Global

| Key | Action |
| --- | --- |
| `P` | Switch between the two rounds |
| `I` | Toggle Shape Inspector |
| `0` | Toggle directional light |
| `7` | Toggle wall lights in Glass Bridge |
| `8` | Toggle point light |
| `9` | Toggle spot light |
| `F3` | Toggle VSync |
| `F4` | Toggle picture-in-picture doll camera |
| `R` | Reset the current round |
| `Esc` | Exit |

### Red Light, Green Light

| Key | Action |
| --- | --- |
| `W` / `S` or `Up` / `Down` | Move forward or backward |
| `A` / `D` or `Left` / `Right` | Turn left or right |
| `Z` / `X` / `C` | Select walk, jog, or sprint speed |
| `1`-`7` | Select a camera |
| Mouse drag / wheel | Orbit / zoom |

### Glass Bridge

| Key | Action |
| --- | --- |
| `A`, `Left`, or `Q` | Jump to the left panel |
| `D`, `Right`, or `E` | Jump to the right panel |
| `Tab` | Toggle fast-forward for AI turns |
| `S` | Skip to Player 456's turn |
| `1`-`5` | Select a camera |

## Project structure

```text
.
|-- assets/              Runtime textures and optional local audio
|-- docs/
|   |-- media/           README media
|   |-- presentation/    Project presentation
|   `-- report/          Report PDF, LaTeX source, and figures
|-- src/
|   |-- audio/           Sound effects and music playback
|   |-- camera/          Round 1 camera controller
|   |-- compat/          Compatibility drawing interface
|   |-- contestants/     Player and NPC simulation
|   |-- core/            Shared configuration and data types
|   |-- doll/            Doll, head, body, and tree models
|   |-- engine/          OpenGL abstractions and primitives
|   |-- environment/     Arena, architecture, and clock
|   |-- glassbridge/     Bridge gameplay, cameras, and physics
|   |-- guards/          Guard models and behavior
|   |-- render/          Scene rendering, lights, and textures
|   |-- shaders/         GLSL vertex and fragment shaders
|   |-- tools/           In-game showcase renderer
|   `-- vendor/          Vendored GLAD, Dear ImGui, and stb_image
|-- tools/               Asset and report-generation scripts
`-- CMakeLists.txt       Portable project build definition
```

Most gameplay tuning values live in [`src/core/Config.h`](src/core/Config.h). Rendering starts in [`src/main_modern.cpp`](src/main_modern.cpp), while the lighting pipeline is implemented in [`src/shaders/default.vert`](src/shaders/default.vert) and [`src/shaders/default.frag`](src/shaders/default.frag).

## Documentation

- [Project report](docs/report/Squid-Game-3D-Project-Report.pdf)
- [Presentation](docs/presentation/Squid-Game-3D-Presentation.pptx)
- [LaTeX report source](docs/report/source/report.tex)

## Regenerating assets and report figures

Generate the procedural textures:

```bash
python tools/make_textures.py
```

Capture the scripted report shots and assemble the figures:

```powershell
.\build\bin\squid_game_opengl.exe --capture docs\report\captures
python tools\make_report_figures.py
```

Python figure generation requires Pillow and NumPy.

## Contributing

Issues and pull requests are welcome. Keep changes focused, follow the existing module boundaries, and verify a clean Release build before opening a pull request. For larger features, open an issue first so the design can be discussed.

## License

No project license has been granted yet. The vendored libraries retain their respective licenses. Add a project-level `LICENSE` before redistributing or reusing the original source code.
