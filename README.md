# Squid Game 3D — Computer Graphics Project

A comprehensive 3D Computer Graphics project built with **C++**, **OpenGL 3.3 Core**, and **Dear ImGui**, featuring:
- **Round 1: Red Light, Green Light** (Colossal Young-hee Doll hugging dead tree, head rotation, Pink Soldiers with weapons, 32 NPC contestants with individual movement and elimination physics).
- **Round 2: Squid Game Glass Bridge** (16-step modular bridge with tempered vs normal glass, 3D shattering shard physics, 16 queued contestants with numbered bibs).
- **3-Type Lighting Lab**: Directional Light (Sun / Stage Floodlight Array), Point Lights (Arena Floodlight Tower / Chandelier, 4 Wall Lamps), and Spot Light (Doll Searchlight / Tracking Follow-Spot).
- **Phong Shading**: lighting (ambient + diffuse + Blinn-Phong specular) is computed for every pixel in the fragment shader.
- **Texture Mapping**: three seamless image textures - sand (arena floor), bark (dead tree) and concrete (glass bridge hall walls & platforms).
- **Shape View (`I`)**: paints every part by its basic shape (box, sphere, cylinder, cone, disk, torus, polygon) to show how each model is built.
- **Hierarchical Modeling & Transformations**: `glPushMatrix`, `glPopMatrix`, `glTranslatef`, `glRotatef`, `glScalef`.

---

## ⚡ Professor Viva & Instant Modification Cheat-Sheet
> **"If your teacher asks you to change anything, here is how you do it in 1 second!"**

| Teacher Asks | File to Open | Variable / Function to Edit |
| :--- | :--- | :--- |
| **Change Tracksuit Color** | [`src/core/Config.h`](file:///d:/4-1/grapgics%20project/src/core/Config.h) | `COLOR_TRACKSUIT = glm::vec3(r, g, b);` |
| **Change Guard Jumpsuit Color** | [`src/core/Config.h`](file:///d:/4-1/grapgics%20project/src/core/Config.h) | `COLOR_GUARD_MAGENTA = glm::vec3(r, g, b);` |
| **Change Sand / Wall / Sky Color** | [`src/core/Config.h`](file:///d:/4-1/grapgics%20project/src/core/Config.h) | `COLOR_ARENA_SAND`, `COLOR_ARENA_WALLS`, `COLOR_ARENA_SKY` |
| **Change Glass Bridge Railing Color** | [`src/core/Config.h`](file:///d:/4-1/grapgics%20project/src/core/Config.h) | `COLOR_GB_PINK_RAIL = glm::vec3(r, g, b);` |
| **Change RLGL Timer (e.g. 60s to 90s)** | [`src/core/Config.h`](file:///d:/4-1/grapgics%20project/src/core/Config.h) | `GAME_TIME_LIMIT = 90.0f;` |
| **Change Glass Bridge Timer** | [`src/core/Config.h`](file:///d:/4-1/grapgics%20project/src/core/Config.h) | `GB_TIME_LIMIT = 960.0f;` |
| **Change Player Movement Speeds** | [`src/core/Config.h`](file:///d:/4-1/grapgics%20project/src/core/Config.h) | `SPEED_SLOW_WALK`, `SPEED_EASY_RUN`, `SPEED_FAST_SPRINT` |
| **Change Glass Bridge Step Count (16)** | [`src/core/Config.h`](file:///d:/4-1/grapgics%20project/src/core/Config.h) | `GB_NUM_STEPS = 16;` |
| **Where is Phong Shading?** | [`src/shaders/default.vert`](file:///d:/4-1/grapgics%20project/src/shaders/default.vert)<br>[`src/shaders/default.frag`](file:///d:/4-1/grapgics%20project/src/shaders/default.frag) | `default.vert`: only transforms vertices.<br>`default.frag`: per-pixel normal re-normalization, diffuse + Blinn-Phong specular for every light. |
| **Where are the Textures?** | [`src/render/SceneTextures.cpp`](file:///d:/4-1/grapgics%20project/src/render/SceneTextures.cpp)<br>[`assets/textures/`](file:///d:/4-1/grapgics%20project/assets/textures) | `GLCompat::setTexture(&SceneTextures::sand(), uRepeat, vRepeat)`; images made by `tools/make_textures.py`. |
| **Make the Glass Bridge hall brighter/darker** | [`src/core/Config.h`](file:///d:/4-1/grapgics%20project/src/core/Config.h)<br>[`src/main_modern.cpp`](file:///d:/4-1/grapgics%20project/src/main_modern.cpp) | `COLOR_GB_WALL`, `COLOR_GB_LAMP`; `ambientIntensity` and the 4 wall-lamp `fill[]` lights in the Glass Bridge lighting block. |
| **Where are the 3 Types of Lighting?** | [`src/core/Types.h`](file:///d:/4-1/grapgics%20project/src/core/Types.h)<br>[`src/render/LightFixtures.cpp`](file:///d:/4-1/grapgics%20project/src/render/LightFixtures.cpp) | Structs in `Types.h`. 3D physical meshes in `LightFixtures.cpp`. |
| **Where is Red Light Green Light logic?** | [`src/contestants/Contestants.cpp`](file:///d:/4-1/grapgics%20project/src/contestants/Contestants.cpp) | `ContestantsManager::update`: Green light walks ($z += \text{speed}$, $\sin(\omega t)$), Red light freezes. |
| **Where is Glass Bridge logic?** | [`src/glassbridge/GlassBridge.cpp`](file:///d:/4-1/grapgics%20project/src/glassbridge/GlassBridge.cpp) | `GlassBridgeManager::update` and `jumpPlayer`: parabolic jump arc, 50/50 safe glass step logic. |

---

## 🏗️ Architecture & Clean File Structure

All code is strictly separated into single-responsibility modules:

```
src/
├── main_modern.cpp           # Clean application entry point, game loops, Dear ImGui HUD
├── core/
│   ├── Config.h              # ⭐ CENTRAL CONFIGURATION (All colors, speeds, dimensions & timers)
│   └── Types.h               # Core data structures (Player, Doll, Guards, NPCs, Lights)
├── shaders/
│   ├── default.vert          # Vertex shader (moves vertices; passes normal, color, UV on)
│   └── default.frag          # Fragment shader (Phong per-pixel lighting, textures, haze)
├── render/
│   ├── Renderer.h / .cpp     # High-level material, primitive and lighting dispatcher
│   ├── SceneTextures.h / .cpp# The 3 image textures (sand, bark, concrete)
│   └── LightFixtures.h / .cpp# 3D physical light meshes (Sun, Chandelier, Spotlight, Gantry, Wall lamps)
├── contestants/
│   └── Contestants.h / .cpp  # Player 456 + NPC contestants, clean movement & elimination
├── doll/
│   ├── Doll.cpp              # Doll manager & 180° head turning state machine
│   ├── DollHead.cpp          # 3D hair, facial features, red scanning eyes
│   ├── DollBody.cpp          # Orange dress, yellow collar shirt, legs, arms
│   └── DollTree.cpp          # Colossal dead gnarled tree trunk & branches
├── guards/
│   └── Guards.h / .cpp       # Pink Soldiers (Circle/Triangle/Square masks, weapons, aiming)
├── environment/
│   ├── Environment.cpp       # Grand arena sand floor, painted sky walls
│   ├── EnvironmentHouses.cpp # Surrounding stadium architecture
│   └── EnvironmentClock.cpp  # Digital 7-segment countdown clock
├── glassbridge/
│   ├── GlassBridge.h         # Glass Bridge subsystem interface
│   ├── GlassBridge.cpp       # Game loop, queue manager, jump physics
│   ├── GlassBridgeEnvironment.cpp # Grand stage walls, ceiling trusses, safety net
│   ├── GlassBridgeSteps.cpp  # Steel I-beams, neon rails, 16 glass panel pairs
│   ├── GlassBridgeContestants.cpp # Waiting queue, 3D bib numerals, guards
│   ├── GlassBridgeShards.cpp # Real-time glass shattering particle physics
│   └── GlassBridgeCamera.cpp # 7 cinematic camera modes (Follow, Runway, Aerial...)
├── camera/
│   └── Camera.h / .cpp       # RLGL 360° orbit and view vectors
├── tools/
│   └── Showcase.h / .cpp     # Renders one model alone (normal + split into shapes) for the report
└── engine/                   # Modern OpenGL Core Profile engine (Shader, VAO, VBO, EBO, Mesh, Texture, GLCompat)

assets/textures/              # sand.png, bark.png, concrete.png
tools/make_textures.py        # Generates the three seamless textures
tools/make_report_figures.py  # Turns captured screenshots into the report's figures
report/                       # Project report (PDF) and its LaTeX source
```

---

## 🎮 Controls & Hotkeys

### Global Hotkeys
- **`I`**: Toggle **Shape View** (color every part by its basic shape kind).
- **`0`**: Toggle **Directional Light** (Sun in RLGL / Stage Floodlight Array in Glass Bridge).
- **`8`**: Toggle **Point Light** (Arena Floodlight Tower in RLGL / Chandelier in Glass Bridge).
- **`7`**: Toggle **Wall Lamps (4x)** (Glass Bridge).
- **`9`**: Toggle **Spot Light** (Doll Eye Searchlight in RLGL / Follow-Spotlight in Glass Bridge).
- **`P`**: Switch Phase between **Round 1 (RLGL)** and **Round 2 (Glass Bridge)**.
- **`F3`**: Toggle **VSync** (uncap framerate for ultra-low input latency).
- **`F4`**: Toggle **CCTV Doll Cam (PiP)**.
- **`R`**: Reset current game round.
- **`Esc`**: Exit application.

### Round 1: Red Light, Green Light
- **W / S or ↑ / ↓**: Walk Forward / Backward.
- **A / D or ← / →**: Turn Left / Right.
- **Z / X / C**: Select walk speed (Precision Walk / Easy Jog / Fast Sprint).
- **1 – 7**: Switch RLGL Cameras (Third-Person, Overhead Free Cam, Aerial Top-Down, Doll Closeup, Stadium Left/Right, Finish Line).
- **Mouse Drag & Scroll**: 360° free orbit and camera zoom.

### Round 2: Glass Bridge
- **A / Left Arrow / Q**: Jump to **Left Glass Panel**.
- **D / Right Arrow / E**: Jump to **Right Glass Panel**.
- **TAB**: Toggle Fast-Forward AI turns.
- **S**: Skip directly to Player 456's turn with instant simulated progression.
- **1 – 5**: Switch Glass Bridge Cameras (Follow, 360° Orbit, Aerial Top-Down, Side Profile, Destination).

---

## 🛠️ Build & Run (MSYS2 MinGW-w64)

```powershell
# Press Ctrl+Shift+B in VS Code, or run directly from powershell:
g++ -O2 -DMODERN_OPENGL `
    -I src/compat -I src -I src/vendor/glad/include -I src/vendor/imgui -I src/vendor/imgui/backends `
    src/main_modern.cpp `
    src/engine/Shader.cpp src/engine/Camera.cpp src/engine/VAO.cpp src/engine/VBO.cpp src/engine/EBO.cpp `
    src/engine/Mesh.cpp src/engine/Primitives.cpp src/engine/GLCompat.cpp src/engine/Texture.cpp `
    src/audio/Audio.cpp src/render/Renderer.cpp src/render/LightFixtures.cpp `
    src/render/SceneTextures.cpp src/tools/Showcase.cpp `
    src/doll/Doll.cpp src/doll/DollHead.cpp src/doll/DollBody.cpp src/doll/DollTree.cpp `
    src/guards/Guards.cpp src/contestants/Contestants.cpp `
    src/environment/Environment.cpp src/environment/EnvironmentHouses.cpp src/environment/EnvironmentClock.cpp `
    src/camera/Camera.cpp `
    src/glassbridge/GlassBridge.cpp src/glassbridge/GlassBridgeEnvironment.cpp `
    src/glassbridge/GlassBridgeSteps.cpp src/glassbridge/GlassBridgeContestants.cpp `
    src/glassbridge/GlassBridgeShards.cpp src/glassbridge/GlassBridgeCamera.cpp `
    src/vendor/glad/src/gl.c src/vendor/imgui/imgui.cpp src/vendor/imgui/imgui_draw.cpp `
    src/vendor/imgui/imgui_tables.cpp src/vendor/imgui/imgui_widgets.cpp `
    src/vendor/imgui/backends/imgui_impl_glfw.cpp src/vendor/imgui/backends/imgui_impl_opengl3.cpp `
    -o bin/game_modern.exe -lglfw3 -lopengl32 -lwinmm -lgdi32

# Run (from the project folder, so shaders and textures are found):
.\bin\game_modern.exe
```

### Regenerating the report screenshots
```powershell
.\bin\game_modern.exe --capture report\captures        # plays ~110 scripted shots, saves .bmp files, quits
.\bin\game_modern.exe --capture report\captures obj_   # optional: only shots whose name starts with "obj_"
python tools\make_report_figures.py                     # converts/combines them into report\source\figures
```
