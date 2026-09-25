# Squid Game 3D — Real-Life Red Light, Green Light

A Computer Graphics course project built with C++, OpenGL, and FreeGLUT featuring authentic Squid Game arena visuals, realistic Young-hee Doll with gnarled tree, Pink Soldiers, NPC contestants, Squid Game audio & chant synthesis, digital 7-segment clock UI, and a modular architecture.

## Architecture & Modular File Structure

Source is organized into one folder per subsystem under `src/`, so any single file stays
small enough to read and debug in isolation:

```
.
├── src/
│   ├── main.cpp                 # Entry point, GLUT callbacks, game loop & state machine
│   ├── core/
│   │   ├── Config.h             # Arena constants, speeds, camera offsets, timings, UI bar widths
│   │   └── Types.h              # Structs for Player, Doll, Guards, NPCs, GameState
│   ├── render/                  # Renderer.h / .cpp — lighting, materials, primitives (cylinders, spheres, boxes)
│   ├── audio/                   # Audio.h / .cpp — WinMM engine & procedural Squid Game sound synthesizer
│   ├── camera/                  # Camera.h / .cpp — 3rd/1st person follow, Doll POV, 360° orbit modes
│   ├── doll/                    # Young-hee Doll, split by concern:
│   │   ├── Doll.h / .cpp        #   state machine (green/red light, head turn) + composition
│   │   ├── DollHead.cpp/.h      #   face, eyes, hair, ears, pigtails
│   │   ├── DollBody.cpp/.h      #   torso, dress, ball-jointed arms/hands, legs
│   │   └── DollTree.cpp/.h      #   the procedural gnarled dead tree
│   ├── guards/                  # Guards.h / .cpp — Pink Soldiers (Circle/Triangle/Square masks, weapons)
│   ├── contestants/             # Contestants.h / .cpp — Player 456 + NPCs, AI logic, blood particles
│   ├── environment/             # Arena, split by concern:
│   │   ├── Environment.h/.cpp   #   ground, sky walls, wheat trim, boundary lines
│   │   ├── EnvironmentHouses.cpp#   background Korean village houses
│   │   └── EnvironmentClock.cpp #   overhead 3D digital scoreboard clock
│   └── ui/                      # HUD, split by concern:
│       ├── UI.h / .cpp          #   sidebar panels, Doll Cam frame, state overlays
│       └── UIPrimitives.h       #   shared 2D drawing toolkit (text, rects, 7-seg digits, timer widget)
├── bin/                    # Build output (game.exe)
├── .vscode/tasks.json      # VS Code build task
└── README.md
```

## Setup & Compilation (Windows)

1. Ensure MSYS2 MinGW-w64 is in your PATH (`C:\msys64\mingw64\bin`).
2. Build via VS Code (`Ctrl+Shift+B`) or run from terminal (`-I src` lets every file use
   folder-qualified includes like `"core/Config.h"`):
   ```powershell
   g++ -I src src/main.cpp src/audio/Audio.cpp src/render/Renderer.cpp `
       src/doll/Doll.cpp src/doll/DollHead.cpp src/doll/DollBody.cpp src/doll/DollTree.cpp `
       src/guards/Guards.cpp src/contestants/Contestants.cpp `
       src/environment/Environment.cpp src/environment/EnvironmentHouses.cpp src/environment/EnvironmentClock.cpp `
       src/ui/UI.cpp src/camera/Camera.cpp `
       -o bin/game.exe -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32
   ```
3. Run the game:
   ```powershell
   .\bin\game.exe
   ```

## Controls

The in-game right sidebar always lists the full, current control scheme during play. Summary:

| Key                    | Action                                              |
|-------------------------|------------------------------------------------------|
| **W/S or ↑/↓**          | Move forward / backward                              |
| **A/D or ←/→**          | Turn left / right                                    |
| **Q / E**               | Strafe left / right                                  |
| **Z / X / C**           | Slow walk / easy run / fast sprint (or hold Shift/Ctrl) |
| **G**                   | Cycle speed gear                                     |
| **1 – 9, 0**             | Switch camera (3rd/1st person, top-down, Doll orbit, stadium, Doll POV, start/finish line, overhead) |
| **Mouse drag / wheel**  | 360° orbit the active camera / zoom                  |
| **F**                   | Toggle flat ↔ smooth shading                          |
| **Space / click**       | Start game from the title screen                     |
| **R**                   | Restart game                                          |
| **Esc**                 | Exit                                                  |

## Features & Graphics Highlights

- **Realistic Young-hee Doll**: Authentic porcelain face, front bangs, twin pigtails with purple ties & clips, yellow polo shirt, orange pinafore dress, white socks, black shoes, 180° head rotation, and glowing red scanning eyes.
- **Dead Gnarled Tree**: Organically branching twisted trunk and bare canopy silhouette standing behind the doll.
- **Real Squid Game Audio Engine**: Procedural 16-bit PCM WAV synthesizers for *"Mugunghwa kkochi pieot seumnida"* chanting with accelerating tempo, sniper gunshots, servo head clicks, digital timer beeps, and fanfare.
- **Squid Game Pink Soldiers**: Hooded magenta jumpsuits, black harnesses, masks with Circle (○), Triangle (△), Square (□) insignias, and MP5 submachine guns with muzzle flash.
- **NPC Contestants & Player 456**: Green numbered tracksuits, dynamic walk cycles, NPC AI with reaction delays and fatal mistakes, and elimination ragdoll physics.
- **Arena Environment**: Sandy terrain, trompe l'œil painted cloud sky perimeter walls, golden wheat field borders, Korean guard houses with green doors and orange tile roofs, and red boundary lines.
- **Authentic UI & Scoreboard**: 7-segment digital countdown clock (`05:00`) in industrial frame, live alive player tracker (`456 / 456`), and Squid Game geometric icons.
