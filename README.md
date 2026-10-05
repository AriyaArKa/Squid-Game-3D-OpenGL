# Squid Game 3D — Real-Life Red Light, Green Light

A Computer Graphics course project built with C++, OpenGL, and FreeGLUT featuring authentic Squid Game arena visuals, realistic Young-hee Doll with gnarled tree, Pink Soldiers, NPC contestants, Squid Game audio & chant synthesis, digital 7-segment clock UI, and a modular architecture.

## Architecture & Modular File Structure

Source is organized into one folder per subsystem under `src/`, so any single file stays
small enough to read and debug in isolation:

```
.
├── src/
│   ├── main_modern.cpp          # Entry point, GLFW window/input loops, Dear ImGui HUD overlays
│   ├── engine/                  # Modern OpenGL engine (Shader, VAO, VBO, EBO, Mesh, Primitives, GLCompat)
│   ├── compat/                  # Compatibility shim mapping legacy GL to Modern OpenGL VAO/VBO
│   ├── shaders/                 # GLSL 330 core vertex & fragment shaders (default.vert, default.frag)
│   ├── core/
│   │   ├── Config.h             # Arena constants, speeds, camera offsets, timings, UI bar widths
│   │   └── Types.h              # Structs for Player, Doll, Guards, NPCs, GameState
│   ├── render/                  # Renderer.h / .cpp — GLCompat material, lighting & primitive dispatch
│   ├── audio/                   # Audio.h / .cpp — WinMM engine & procedural Squid Game sound synthesizer
│   ├── camera/                  # Camera.h / .cpp — View vectors, Doll POV, 360° orbit modes
│   ├── doll/                    # Young-hee Doll (Doll.cpp, DollHead.cpp, DollBody.cpp, DollTree.cpp)
│   ├── guards/                  # Guards.h / .cpp — Pink Soldiers (Circle/Triangle/Square masks, weapons)
│   ├── contestants/             # Contestants.h / .cpp — Player 456 + NPCs, AI logic, blood particles
│   ├── environment/             # Arena (Environment.cpp, EnvironmentHouses.cpp, EnvironmentClock.cpp)
│   └── vendor/                  # GLAD (OpenGL loader) and Dear ImGui
├── bin/                         # Build output (game_modern.exe)
├── .vscode/tasks.json           # VS Code build task
└── README.md
```

## Setup & Compilation (Windows)

1. Ensure MSYS2 MinGW-w64 is in your PATH (`C:\msys64\mingw64\bin`).
2. Build via VS Code (`Ctrl+Shift+B`) or run from terminal:
   ```powershell
   g++ -O2 -DMODERN_OPENGL `
       -I src/compat -I src -I src/vendor/glad/include -I src/vendor/imgui -I src/vendor/imgui/backends `
       src/main_modern.cpp `
       src/engine/Shader.cpp src/engine/Camera.cpp src/engine/VAO.cpp src/engine/VBO.cpp src/engine/EBO.cpp `
       src/engine/Mesh.cpp src/engine/Primitives.cpp src/engine/GLCompat.cpp `
       src/audio/Audio.cpp src/render/Renderer.cpp `
       src/doll/Doll.cpp src/doll/DollHead.cpp src/doll/DollBody.cpp src/doll/DollTree.cpp `
       src/guards/Guards.cpp src/contestants/Contestants.cpp `
       src/environment/Environment.cpp src/environment/EnvironmentHouses.cpp src/environment/EnvironmentClock.cpp `
       src/camera/Camera.cpp src/glassbridge/GlassBridge.cpp src/vendor/glad/src/gl.c `
       src/vendor/imgui/imgui.cpp src/vendor/imgui/imgui_draw.cpp src/vendor/imgui/imgui_tables.cpp `
       src/vendor/imgui/imgui_widgets.cpp src/vendor/imgui/backends/imgui_impl_glfw.cpp src/vendor/imgui/backends/imgui_impl_opengl3.cpp `
       -o bin/game_modern.exe -lglfw3 -lopengl32 -lwinmm -lgdi32
   ```
3. Run the game:
   ```powershell
   .\bin\game_modern.exe
   ```

## Squid Game Modes & Controls

Switch between game phases at any time via the top navigation bar or the **P** hotkey. When winning Round 1, Player 456 seamlessly advances to Round 2!

### Round 1: Red Light, Green Light
- **W / S or ↑ / ↓**: Move forward / backward
- **A / D or ← / →**: Turn left / right
- **Z / X / C**: Slow walk / easy jog / fast sprint (or hold Shift / Ctrl)
- **1 – 7**: Switch camera (3rd person, overhead free cam, top-down, Doll closeup, stadium, finish line)
- **Mouse Drag / Wheel**: 360° orbit active camera / zoom in & out
- **P**: Switch to Round 2: Glass Bridge | **R**: Restart round

### Round 2: Squid Game Glass Bridge
Crossing the suspended 16-step glass bridge high above the bottomless abyss (matching Episode 7!).
- **Random Queue Assignment**: Player 456 is assigned a random queue position (1 to 16) with an authentic Squid Game bib.
- **AI Turn Progression**: Contestants ahead of you in the queue take their turns one by one, picking Left or Right.
- **Tempered vs Normal Glass**: Stepping on safe tempered glass holds your weight; stepping on fragile normal glass shatters into 3D sparkling shards and sends the jumper plummeting into the abyss!
- **Interactive Controls**:
  - **A / Left Arrow / Q** (or GUI button): Jump to Left Glass panel
  - **D / Right Arrow / E** (or GUI button): Jump to Right Glass panel
  - **Tab / F**: Toggle Fast-Forward AI turns
  - **1 – 7**: Switch between all 7 camera angles:
    - **[1] 3rd Person Follow**: Smooth camera following active jumper
    - **[2] Bridge Runway View (Episode 7)**: Iconic runway perspective looking straight down the illuminated bridge (Image 1)
    - **[3] Queue Over-the-Shoulder**: Dramatic view from behind the waiting contestants (Image 2)
    - **[4] Free 360° Orbit**: Full mouse drag rotation and wheel zoom
    - **[5] Top-Down Aerial**: Direct plan view of the 16 steps and the chasm
    - **[6] Side Profile Broadcast**: Stadium lateral camera showing leap arcs
    - **[7] Destination Cam**: Platform camera looking back toward incoming jumpers
  - **P**: Switch back to Round 1: Red Light, Green Light | **R**: Restart Glass Bridge

### 💡 Computer Graphics Lab Features: 3-Type Lighting & Shading
- **[0] Directional Light Toggle**: Toggles global directional sunlight / stage key lighting.
- **[8] Point Light Toggle**: Toggles omnidirectional point light with quadratic distance attenuation ($1 / (k_c + k_l d + k_q d^2)$).
  - *Round 1*: High arena floodlight pylon tower with physical steel tower and glowing lamp head.
  - *Round 2*: Vintage brass stage chandelier suspended above the central glass bridge steps.
- **[9] Spot Light Toggle**: Toggles focused spotlight with inner/outer angular cone falloff ($\theta = L \cdot -D$).
  - *Round 1*: Young-hee Doll eye scanning searchlight beam that swivels dynamically with her head rotation!
  - *Round 2*: Theatrical follow-spotlight fixture and volumetric light cone tracking the active jumping contestant.
- **[F] Shading Model Switch (Gouraud vs Phong)**:
  - **Phong Shading (Per-Pixel)**: Interpolates surface normals to calculate Blinn-Phong specular and diffuse lighting per fragment.
  - **Gouraud Shading (Per-Vertex)**: Evaluates all 3 light sources per vertex in the vertex shader and interpolates colors across polygons.
  - Full interactive checkboxes, mode buttons, and status indicators in the Dear ImGui right panel!

## Features & Graphics Highlights

- **Realistic Young-hee Doll**: Authentic porcelain face, front bangs, twin pigtails with purple ties & clips, yellow polo shirt, orange pinafore dress, white socks, black shoes, 180° head rotation, and glowing red scanning eyes.
- **Dead Gnarled Tree**: Organically branching twisted trunk and bare canopy silhouette standing behind the doll.
- **Real Squid Game Audio Engine**: Procedural 16-bit PCM WAV synthesizers for *"Mugunghwa kkochi pieot seumnida"* chanting with accelerating tempo, sniper gunshots, servo head clicks, digital timer beeps, and fanfare.
- **Squid Game Pink Soldiers**: Hooded magenta jumpsuits, black harnesses, masks with Circle (○), Triangle (△), Square (□) insignias, and MP5 submachine guns with muzzle flash.
- **NPC Contestants & Player 456**: Green numbered tracksuits, dynamic walk cycles, NPC AI with reaction delays and fatal mistakes, and elimination ragdoll physics.
- **Arena Environment**: Sandy terrain, trompe l'œil painted cloud sky perimeter walls, golden wheat field borders, Korean guard houses with green doors and orange tile roofs, and red boundary lines.
- **Authentic UI & Scoreboard**: 7-segment digital countdown clock (`05:00`) in industrial frame, live alive player tracker (`456 / 456`), and Squid Game geometric icons.
