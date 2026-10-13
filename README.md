# Squid Game: OpenGL Trials

<p align="center">
  <img src="docs/media/banner.jpg" alt="Gameplay showcase of the Squid Game OpenGL Trials arena" title="Squid Game: OpenGL Trials gameplay showcase" width="900">
</p>

<p align="center"><em>Figure 1 - Real-time OpenGL rendering with the doll, contestants, arena lighting, and environment models.</em></p>

## Feature tour

[![Watch the Squid Game: OpenGL Trials feature tour](docs/media/banner.jpg)](docs/media/feature-tour.mp4 "Play the feature tour video")
<p align="center"><em>Click the image above to play or open the full feature tour video.</em></p>

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

## Visual gallery

The gallery below contains the figures generated for the project report. Every image is linked from the repository, so visitors can inspect the complete game at a glance without downloading the PDF.

### Gameplay, environments, and interface

<table>
<tr>
<td><img src="docs/report/source/figures/rlgl_wide.jpg" alt="Wide view of the Red Light, Green Light arena" width="240"></td>
<td><img src="docs/report/source/figures/gb_hall.jpg" alt="Glass Bridge hall environment" width="240"></td>
<td><img src="docs/report/source/figures/gb_sides.jpg" alt="Glass Bridge side view" width="240"></td>
<td><img src="docs/report/source/figures/gb_shatter.jpg" alt="Glass panel shattering effect" width="240"></td>
</tr>
<tr>
<td><img src="docs/report/source/figures/ui_rlgl_start.jpg" alt="Red Light, Green Light start screen" width="240"></td>
<td><img src="docs/report/source/figures/ui_rlgl_green.jpg" alt="Green light gameplay HUD" width="240"></td>
<td><img src="docs/report/source/figures/ui_rlgl_red.jpg" alt="Red light gameplay HUD" width="240"></td>
<td><img src="docs/report/source/figures/ui_rlgl_won.jpg" alt="Red Light, Green Light victory screen" width="240"></td>
</tr>
<tr>
<td><img src="docs/report/source/figures/ui_gb_turn.jpg" alt="Glass Bridge player turn HUD" width="240"></td>
<td><img src="docs/report/source/figures/ui_gb_won.jpg" alt="Glass Bridge victory screen" width="240"></td>
<td><img src="docs/report/source/figures/basic_shapes.png" alt="Basic geometric primitives used by the renderer" width="240"></td>
<td><img src="docs/report/source/figures/kuet_logo.png" alt="KUET project report logo" width="240"></td>
</tr>
</table>

### Lighting and shading

<table>
<tr>
<td><img src="docs/report/source/figures/lights_rlgl.jpg" alt="Red Light, Green Light lighting setup" width="300"></td>
<td><img src="docs/report/source/figures/lights_gb.jpg" alt="Glass Bridge lighting setup" width="300"></td>
<td><img src="docs/report/source/figures/phong_doll.jpg" alt="Phong shading on the Young-hee doll" width="300"></td>
<td><img src="docs/report/source/figures/phong_players.jpg" alt="Phong shading on player models" width="300"></td>
</tr>
</table>

### Textures and materials

<table>
<tr>
<td><img src="docs/report/source/figures/textures.png" alt="Project texture atlas overview" width="300"></td>
<td><img src="docs/report/source/figures/tex_sand.jpg" alt="Sand floor texture" width="300"></td>
<td><img src="docs/report/source/figures/tex_tree.jpg" alt="Bark texture used on the tree" width="300"></td>
<td><img src="docs/report/source/figures/tex_wall.jpg" alt="Concrete wall texture" width="300"></td>
</tr>
</table>

### Model construction and decomposition

<table>
<tr>
<td><img src="docs/report/source/figures/decomp_arena.jpg" alt="Arena decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_hall.jpg" alt="Glass Bridge hall decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_doll.png" alt="Doll model decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_dollhead.png" alt="Doll head decomposition" width="240"></td>
</tr>
<tr>
<td><img src="docs/report/source/figures/decomp_tree.png" alt="Dead tree decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_player.png" alt="Player model decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_guard.png" alt="Guard model decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_gbguard.png" alt="Glass Bridge guard decomposition" width="240"></td>
</tr>
<tr>
<td><img src="docs/report/source/figures/decomp_house.png" alt="House decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_clock.png" alt="Countdown clock decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_bridge.png" alt="Bridge decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_platform.png" alt="Platform decomposition" width="240"></td>
</tr>
<tr>
<td><img src="docs/report/source/figures/decomp_chandelier.png" alt="Chandelier decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_floodlight.png" alt="Floodlight decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_spotlight.png" alt="Spotlight decomposition" width="240"></td>
<td><img src="docs/report/source/figures/decomp_gantry.png" alt="Gantry decomposition" width="240"></td>
</tr>
<tr>
<td><img src="docs/report/source/figures/decomp_walllamp.png" alt="Wall lamp decomposition" width="240"></td>
</tr>
</table>

### Camera and model turntables

<table>
<tr>
<td><img src="docs/report/source/figures/cameras_rlgl.jpg" alt="Red Light, Green Light camera lineup" width="300"></td>
<td><img src="docs/report/source/figures/cameras_gb.jpg" alt="Glass Bridge camera lineup" width="300"></td>
<td><img src="docs/report/source/figures/views_doll.png" alt="Doll model turntable" width="300"></td>
<td><img src="docs/report/source/figures/views_bridge.png" alt="Bridge model turntable" width="300"></td>
</tr>
<tr>
<td><img src="docs/report/source/figures/views_player.png" alt="Player model turntable" width="300"></td>
<td><img src="docs/report/source/figures/views_guard.png" alt="Guard model turntable" width="300"></td>
<td><img src="docs/report/source/figures/views_tree.png" alt="Tree model turntable" width="300"></td>
<td><img src="docs/report/source/figures/views_house.png" alt="House model turntable" width="300"></td>
</tr>
<tr>
<td><img src="docs/report/source/figures/views_dollhead.png" alt="Doll head turntable" width="300"></td>
<td><img src="docs/report/source/figures/views_clock.png" alt="Clock turntable" width="300"></td>
<td><img src="docs/report/source/figures/views_platform.png" alt="Platform turntable" width="300"></td>
<td><img src="docs/report/source/figures/views_gbguard.png" alt="Glass Bridge guard turntable" width="300"></td>
</tr>
<tr>
<td><img src="docs/report/source/figures/views_chandelier.png" alt="Chandelier turntable" width="300"></td>
<td><img src="docs/report/source/figures/views_floodlight.png" alt="Floodlight turntable" width="300"></td>
<td><img src="docs/report/source/figures/views_spotlight.png" alt="Spotlight turntable" width="300"></td>
<td><img src="docs/report/source/figures/views_gantry.png" alt="Gantry turntable" width="300"></td>
</tr>
<tr>
<td><img src="docs/report/source/figures/views_walllamp.png" alt="Wall lamp turntable" width="300"></td>
</tr>
</table>

<details>
<summary><strong>Open all orthographic model views</strong></summary>

The report also includes front, side, top, and isometric views for every modeled asset. These links preserve the complete technical reference without making the landing page excessively tall.

<table>
<tr>
<td><a href="docs/report/source/figures/view_doll_front.png">Doll front</a> · <a href="docs/report/source/figures/view_doll_side.png">side</a> · <a href="docs/report/source/figures/view_doll_top.png">top</a> · <a href="docs/report/source/figures/view_doll_iso.png">isometric</a></td>
<td><a href="docs/report/source/figures/view_dollhead_front.png">Doll head front</a> · <a href="docs/report/source/figures/view_dollhead_side.png">side</a> · <a href="docs/report/source/figures/view_dollhead_top.png">top</a> · <a href="docs/report/source/figures/view_dollhead_iso.png">isometric</a></td>
<td><a href="docs/report/source/figures/view_tree_front.png">Tree front</a> · <a href="docs/report/source/figures/view_tree_side.png">side</a> · <a href="docs/report/source/figures/view_tree_top.png">top</a> · <a href="docs/report/source/figures/view_tree_iso.png">isometric</a></td>
</tr>
<tr>
<td><a href="docs/report/source/figures/view_player_front.png">Player front</a> · <a href="docs/report/source/figures/view_player_side.png">side</a> · <a href="docs/report/source/figures/view_player_top.png">top</a> · <a href="docs/report/source/figures/view_player_iso.png">isometric</a></td>
<td><a href="docs/report/source/figures/view_guard_front.png">Guard front</a> · <a href="docs/report/source/figures/view_guard_side.png">side</a> · <a href="docs/report/source/figures/view_guard_top.png">top</a> · <a href="docs/report/source/figures/view_guard_iso.png">isometric</a></td>
<td><a href="docs/report/source/figures/view_gbguard_front.png">Bridge guard front</a> · <a href="docs/report/source/figures/view_gbguard_side.png">side</a> · <a href="docs/report/source/figures/view_gbguard_top.png">top</a> · <a href="docs/report/source/figures/view_gbguard_iso.png">isometric</a></td>
</tr>
<tr>
<td><a href="docs/report/source/figures/view_house_front.png">House front</a> · <a href="docs/report/source/figures/view_house_side.png">side</a> · <a href="docs/report/source/figures/view_house_top.png">top</a> · <a href="docs/report/source/figures/view_house_iso.png">isometric</a></td>
<td><a href="docs/report/source/figures/view_clock_front.png">Clock front</a> · <a href="docs/report/source/figures/view_clock_side.png">side</a> · <a href="docs/report/source/figures/view_clock_top.png">top</a> · <a href="docs/report/source/figures/view_clock_iso.png">isometric</a></td>
<td><a href="docs/report/source/figures/view_bridge_front.png">Bridge front</a> · <a href="docs/report/source/figures/view_bridge_side.png">side</a> · <a href="docs/report/source/figures/view_bridge_top.png">top</a> · <a href="docs/report/source/figures/view_bridge_iso.png">isometric</a></td>
</tr>
<tr>
<td><a href="docs/report/source/figures/view_platform_front.png">Platform front</a> · <a href="docs/report/source/figures/view_platform_side.png">side</a> · <a href="docs/report/source/figures/view_platform_top.png">top</a> · <a href="docs/report/source/figures/view_platform_iso.png">isometric</a></td>
<td><a href="docs/report/source/figures/view_chandelier_front.png">Chandelier front</a> · <a href="docs/report/source/figures/view_chandelier_side.png">side</a> · <a href="docs/report/source/figures/view_chandelier_top.png">top</a> · <a href="docs/report/source/figures/view_chandelier_iso.png">isometric</a></td>
<td><a href="docs/report/source/figures/view_floodlight_front.png">Floodlight front</a> · <a href="docs/report/source/figures/view_floodlight_side.png">side</a> · <a href="docs/report/source/figures/view_floodlight_top.png">top</a> · <a href="docs/report/source/figures/view_floodlight_iso.png">isometric</a></td>
</tr>
<tr>
<td><a href="docs/report/source/figures/view_spotlight_front.png">Spotlight front</a> · <a href="docs/report/source/figures/view_spotlight_side.png">side</a> · <a href="docs/report/source/figures/view_spotlight_top.png">top</a> · <a href="docs/report/source/figures/view_spotlight_iso.png">isometric</a></td>
<td><a href="docs/report/source/figures/view_gantry_front.png">Gantry front</a> · <a href="docs/report/source/figures/view_gantry_side.png">side</a> · <a href="docs/report/source/figures/view_gantry_top.png">top</a> · <a href="docs/report/source/figures/view_gantry_iso.png">isometric</a></td>
<td><a href="docs/report/source/figures/view_walllamp_front.png">Wall lamp front</a> · <a href="docs/report/source/figures/view_walllamp_side.png">side</a> · <a href="docs/report/source/figures/view_walllamp_top.png">top</a> · <a href="docs/report/source/figures/view_walllamp_iso.png">isometric</a></td>
</tr>
</table>
</details>

## Rendering details

### Shading pipeline

The renderer uses a modern OpenGL 3.3 Core pipeline. Each mesh provides position, normal, colour, and UV attributes. The vertex shader transforms positions into clip space and carries world-space position, a correctly transformed normal, vertex colour, and texture coordinates to the fragment stage. The fragment shader then performs per-pixel lighting:

1. **Ambient term** keeps unlit surfaces readable.
2. **Diffuse Lambert term** responds to the angle between the surface normal and each light direction.
3. **Blinn-Phong specular term** produces focused highlights using the halfway vector between the light and camera.
4. **Material colour and texture sampling** are combined before distance haze and alpha handling.

This keeps the same material system usable for the arena, characters, props, bridge glass, and UI showcase scenes. The implementation is in [`default.vert`](src/shaders/default.vert) and [`default.frag`](src/shaders/default.frag).

### Lighting system

The game exposes independent light families that can be toggled while running:

| Light type | Scene role | Behaviour |
| --- | --- | --- |
| Directional | Arena sun / Glass Bridge floodlight array | Parallel rays for broad daylight or stage illumination |
| Point | Arena floodlight tower / Bridge chandelier | Local inverse-distance illumination with bright falloff |
| Wall fill | Four Glass Bridge wall lamps | Low-intensity local fill that prevents black corners |
| Spot | Doll eye searchlight / tracking follow-spot | Cone-limited light aimed at the target or active subject |

The physical fixtures are modeled in `src/render/LightFixtures.cpp`; shared light data lives in `src/core/Types.h`; scene-specific intensity and colour values are centralized in `src/core/Config.h`. Use `0`, `7`, `8`, and `9` to compare the lighting contributions live.

### Camera system

Round 1 provides seven views: third-person gameplay, overhead free camera, aerial top-down, doll close-up, stadium left, stadium right, and finish line. Round 2 provides five: player follow, 360-degree orbit, aerial top-down, side profile, and destination view. Mouse drag and the scroll wheel control orbit and zoom in the free camera. `F4` enables the picture-in-picture doll camera, while number keys switch the active cinematic shot.

### Texture pipeline

The project ships three seamless PNG textures in [`assets/textures`](assets/textures): sand for the arena floor, bark for the dead tree, and concrete for the Glass Bridge hall and platforms. `SceneTextures` loads them through the vendored `stb_image` decoder, and the renderer applies configurable UV repeat values so a small texture can cover large surfaces without stretching. The generated texture source is [`tools/make_textures.py`](tools/make_textures.py); the material examples are shown in the gallery above.

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
