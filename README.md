# Squid Game 3D — Real-Life Red Light, Green Light

A Computer Graphics course project built with C++, OpenGL, and FreeGLUT featuring authentic Squid Game arena visuals, realistic Young-hee Doll with gnarled tree, Pink Soldiers, NPC contestants, Squid Game audio & chant synthesis, digital 7-segment clock UI, and a modular architecture.

## Architecture & Modular File Structure

```
.
├── src/
│   ├── Config.h            # Arena constants, speeds, camera offsets, timings
│   ├── Types.h             # Structs for Player, Doll, Guards, NPCs, GameState
│   ├── Audio.h / .cpp      # WinMM audio engine & procedural Squid Game sound synthesizer
│   ├── Renderer.h / .cpp   # OpenGL lighting, materials, primitives (cylinders, spheres, boxes)
│   ├── Doll.h / .cpp       # Young-hee Doll (pigtails, clips, collared dress, laser eyes) & dead tree
│   ├── Guards.h / .cpp     # Pink Soldiers (Circle, Triangle, Square masks, weapons, muzzle flash)
│   ├── Contestants.h / .cpp# Player 456 + NPC tracksuits, AI running/freeze logic, blood particles
│   ├── Environment.h / .cpp# Sandy arena ground, painted sky backdrop walls, guard huts, lines, 3D clock
│   ├── UI.h / .cpp         # Authentic 7-segment digital countdown clock, player counter, HUD & screens
│   ├── Camera.h / .cpp     # Smooth 3rd person follow, 1st person POV, Doll POV modes
│   └── main.cpp            # Entry point, GLUT callbacks, game loop & state machine
├── bin/                    # Build output (game.exe)
├── .vscode/tasks.json      # VS Code build task
└── README.md
```

## Setup & Compilation (Windows)

1. Ensure MSYS2 MinGW-w64 is in your PATH (`C:\msys64\mingw64\bin`).
2. Build via VS Code (`Ctrl+Shift+B`) or run from terminal:
   ```powershell
   g++ src/*.cpp -o bin/game.exe -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32
   ```
3. Run the game:
   ```powershell
   .\bin\game.exe
   ```

## Controls

| Key           | Action                                       |
|---------------|----------------------------------------------|
| **W / Up**    | Move forward                                 |
| **S / Down**  | Move backward                                |
| **A / D**     | Turn left / right                            |
| **Q / E**     | Strafe left / right                          |
| **C**         | Switch Camera (3rd Person / 1st Person / Doll POV) |
| **F**         | Toggle Flat <-> Smooth Gouraud shading       |
| **Space**     | Start game from title screen                 |
| **R**         | Restart game                                 |
| **Esc**       | Exit                                         |

## Features & Graphics Highlights

- **Realistic Young-hee Doll**: Authentic porcelain face, front bangs, twin pigtails with purple ties & clips, yellow polo shirt, orange pinafore dress, white socks, black shoes, 180° head rotation, and glowing red scanning eyes.
- **Dead Gnarled Tree**: Organically branching twisted trunk and bare canopy silhouette standing behind the doll.
- **Real Squid Game Audio Engine**: Procedural 16-bit PCM WAV synthesizers for *"Mugunghwa kkochi pieot seumnida"* chanting with accelerating tempo, sniper gunshots, servo head clicks, digital timer beeps, and fanfare.
- **Squid Game Pink Soldiers**: Hooded magenta jumpsuits, black harnesses, masks with Circle (○), Triangle (△), Square (□) insignias, and MP5 submachine guns with muzzle flash.
- **NPC Contestants & Player 456**: Green numbered tracksuits, dynamic walk cycles, NPC AI with reaction delays and fatal mistakes, and elimination ragdoll physics.
- **Arena Environment**: Sandy terrain, trompe l'œil painted cloud sky perimeter walls, golden wheat field borders, Korean guard houses with green doors and orange tile roofs, and red boundary lines.
- **Authentic UI & Scoreboard**: 7-segment digital countdown clock (`05:00`) in industrial frame, live alive player tracker (`456 / 456`), and Squid Game geometric icons.
