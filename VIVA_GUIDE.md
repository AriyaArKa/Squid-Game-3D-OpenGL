# SQUID GAME 3D — VIVA GUIDE

Everything you need to explain the project and to find anything in the code in under 10 seconds.
Read Part 0 and Part 1 first. Then read the part for whatever the teacher asks.

Tip for the viva: in VS Code press **Ctrl+Shift+F** (search in all files) and type the
keyword written in `like this` below. Every section tells you the keyword and the file.

- - -

## PART 0 — THE 60-SECOND STORY (say this first)

"My project is a 3D Squid Game made with C++ and modern OpenGL 3.3.
It has two rounds: Red Light Green Light and the Glass Bridge.
**Every object is built only from basic shapes** (cube, sphere, cylinder, cone, disk, torus,
custom triangles). I place each shape with **translate, rotate and scale**, and I group shapes
with **push/pop matrix** so a child part follows its parent (hierarchy).
Everything is drawn by **one shader pair**: the vertex shader moves the vertices, and the
fragment shader does **Phong (Blinn-Phong) lighting for every pixel**.
I have **3 light types** (directional, point, spot), **3 textures** (sand, bark, concrete),
**12 camera modes**, and **keyboard + mouse + ImGui panel** interaction."

- - -

## PART 1 — MAP OF THE PROJECT (what file does what)

```
src/main_modern.cpp      THE MAIN FILE. window, game loop, input, lighting setup, ImGui panels
src/core/Config.h        all numbers: colors, speeds, sizes, timers (change things here)
src/core/Types.h         all structs: Player, Doll, NPCContestant, Guard, lights, enums
src/shaders/default.vert vertex shader  (moves vertices)
src/shaders/default.frag fragment shader (Phong lighting + texture + fog)  <-- the "shading" file
src/engine/              low-level OpenGL wrappers:
    Shader.cpp             compile/link shader, set uniforms
    VAO/VBO/EBO.cpp        OpenGL buffer objects
    Mesh.cpp               one VAO+VBO+EBO = one drawable shape
    Primitives.cpp         BUILDS the cube, sphere, cylinder, disk, torus meshes (vertex lists)
    GLCompat.cpp           matrix stack (push/pop/translate/rotate/scale) + draw functions
    Texture.cpp            loads PNG with stb_image into an OpenGL texture
    Camera.cpp             lookAt (view) + perspective (projection) matrices
src/compat/GL/glut.h     makes glPushMatrix()/glTranslatef()/glutSolidSphere() call GLCompat
src/render/Renderer.cpp  Renderer::drawBox/drawSphere/setMaterial -> forwards to GLCompat
src/render/SceneTextures loads the 3 textures
src/render/LightFixtures draws the physical lamps (sun orb, tower, chandelier, spotlight, wall lamps)
src/doll/                Doll.cpp (state machine) DollBody.cpp DollHead.cpp (big face) DollTree.cpp
src/contestants/         Player 456 + 32 NPCs: drawing, walking, elimination, blood particles
src/guards/              pink soldiers: masks, gun, aiming
src/environment/         sand floor, painted sky walls, houses, 7-segment clock
src/glassbridge/         Round 2: bridge, steps, AI jumpers, shards, cameras
src/camera/Camera.cpp    Round 1 camera modes (CameraManager)
src/audio/Audio.cpp      sound effects made in code + the squid_song.mp3
src/tools/Showcase.cpp   draws ONE model alone (used for report/slides figures)
assets/textures/*.png    sand.png, bark.png, concrete.png  (made by tools/make_textures.py)
```

### "Where is it?" — instant answers

| Teacher asks | Open this file | Search for |
| ------------ | -------------- | ---------- |
| Where is Phong shading? | src/shaders/default.frag | `addLight` , `void main` |
| Where does the vertex shader run? | src/shaders/default.vert | `gl_Position` |
| Where is the cube/sphere created? | src/engine/Primitives.cpp | `buildBox` , `buildSphere` |
| Where are translate/rotate/scale? | src/engine/GLCompat.cpp | `GLCompat::translatef` |
| Where is hierarchy (push/pop)? | any draw file, e.g. src/doll/DollBody.cpp | `glPushMatrix` |
| Where is the doll's face? | src/doll/DollHead.cpp | `drawHead` |
| Where is the tree? | src/doll/DollTree.cpp | `drawDeadTree` , `drawTwig` |
| Where is the player model? | src/contestants/Contestants.cpp | `drawContestantFigure` |
| Where is the camera? | src/camera/Camera.cpp, src/engine/Camera.cpp | `gluLookAt` , `SetView` |
| Where are the 3 lights set? | src/main\_modern.cpp | `DYNAMIC 3-TYPE LIGHTING` |
| Where are lights sent to GPU? | src/engine/GLCompat.cpp | `beginFrame` |
| Where are the textures applied? | DollTree.cpp, Environment.cpp, GlassBridgeEnvironment.cpp | `setTexture` |
| Where is keyboard input? | src/main\_modern.cpp | `keyCallback` |
| Where is mouse input? | src/main\_modern.cpp | `cursorPosCallback` , `scrollCallback` |
| Where is the UI panel? | src/main\_modern.cpp | `ImGui::Begin` |
| Where is the main loop? | src/main\_modern.cpp | `MAIN GAME LOOP` |
| Where is red/green light logic? | src/doll/Doll.cpp | `DollManager::update` |
| Where do NPCs die? | src/contestants/Contestants.cpp | `ContestantsManager::update` |
| Where is glass breaking? | src/glassbridge/GlassBridge.cpp + GlassBridgeShards.cpp | `spawnShards` |
| Where is Shape View (key I)? | src/engine/GLCompat.cpp | `colorByShape` |
| Where are colors/speeds/timers? | src/core/Config.h | the constant name |

- - -

## PART 2 — HOW THE PROGRAM RUNS (main\_modern.cpp)

`main()` starts at the line `int main(int argc, char** argv)` (\~line 669).

**Startup (runs once)**

1. `glfwInit()` and `glfwCreateWindow` — GLFW makes the window. Hints ask for OpenGL **3.3 Core**.
2. `gladLoadGL` — GLAD loads the OpenGL functions.
3. `glEnable(GL_DEPTH_TEST)` — nearer things hide farther things.
4. `Shader shader("src/shaders/default.vert", "src/shaders/default.frag")` — compiles the shaders.
5. `Camera camera(...)` — the camera object.
6. `SceneTextures::loadAll()` — loads sand, bark, concrete PNGs.
7. `AudioManager::init()`, `EnvironmentManager::init()`, `resetRLGL()`, `resetGlassBridge()`.
8. ImGui is created (the side panels).
9. Callbacks registered: key, mouse button, cursor, scroll, resize.

**Every frame** (inside `while (!glfwWindowShouldClose(window))`)

1. `glfwPollEvents()` — read keyboard/mouse.
2. `dt` = time since last frame (clamped to 0.05). Everything moves by `speed * dt`, so the
game runs the same on slow and fast computers (frame-rate independent).
3. **UPDATE** (logic): `DollManager::update`, `GuardsManager::update`,
`ContestantsManager::update`, player movement, `CameraManager::update`, or in Round 2
`GlassBridgeManager::update`.
4. **PASS 1 — draw the 3D scene** in the center viewport (`glViewport(leftBarW, 0, centerW, ...)`):
    * compute camera (`camera.SetView(...)`)
    * set up the 3 lights (`sceneLighting...`)
    * `GLCompat::beginFrame(shader, camera, sceneLighting)` — uploads camera + lights to the shader
    * draw everything: environment, doll, guards, NPCs, player, particles, light fixtures
5. **PASS 2 — ImGui panels** (left HUD, right controls).
6. **PASS 3 — CCTV doll-cam**: renders a second small picture of the doll's face into a
rectangle (`glScissor` + `glViewport`).
7. `glfwSwapBuffers` — show the finished picture.

**Say:** "Update first, then draw. The same loop handles both rounds; a variable
`currentPhase` decides which round's update and draw code runs."

- - -

## PART 3 — THE ENGINE LAYER (how a shape really gets on the screen)

### 3.1 What is a mesh?

A shape = a list of **vertices** + a list of **indices** (which vertices make each triangle).
Each vertex has 4 things (`struct Vertex`): **position, normal, color, texture coordinate (UV)**.

### 3.2 VAO, VBO, EBO (src/engine/Mesh.cpp)

* **VBO** (vertex buffer): the vertex data copied to GPU memory.
* **EBO** (element buffer): the index list on the GPU.
* **VAO** (vertex array): remembers *how* to read the VBO — 4 `LinkAttrib` lines:
location 0 = position, 1 = normal, 2 = color, 3 = texcoord. These match
`layout (location = N)` in default.vert.
* Drawing = `vao.Bind(); glDrawElements(GL_TRIANGLES, ...)`.

### 3.3 How the basic shapes are created (src/engine/Primitives.cpp)

Each shape is built **once** (cached with `static`) as a **unit** shape, then reused thousands of times:

* **Cube** `buildBox()`: 6 faces x 4 corners = 24 vertices, from -0.5 to +0.5. Each face has its
own 4 vertices so it keeps a **flat normal** (sharp edges). 2 triangles per face = 12 triangles.
* **Sphere** `buildSphere()`: a grid of `stacks` (latitude) x `slices` (longitude).
`phi` goes -90..+90, `theta` goes 0..360.
position = (cos(phi)cos(theta), sin(phi), cos(phi)sin(theta)).
For a unit sphere **the normal equals the position** (that is why `v.push_back({pos, pos, ...})`).
* **Cylinder** `cylinder(...)`: ring of vertices at bottom and top + cap disks. Cached by its
parameters in a `std::map`.
* **Disk/ring** `disk(...)`: flat ring between inner and outer radius.
* **Torus** `torus(...)`: a circle (tube) swept around a bigger circle (ring).
* **Cone** = cylinder with top radius 0.
* **Custom polygons** (pleated skirt, sky walls): drawn with `glBegin/glVertex3f/glEnd`,
which `GLCompat::beginPrimitive/vertex3f/endPrimitive` collects into a mesh.

**Say:** "I never load a 3D model file. A unit cube is scaled into an arm, a unit sphere is
scaled into a head. Only 7 meshes exist on the GPU."

### 3.4 The matrix stack (src/engine/GLCompat.cpp)

`GLCompat` keeps a `std::vector<glm::mat4> matrixStack`.

* `pushMatrix()` : copy the top matrix and push it (save the parent's position).
* `popMatrix()` : remove the top (go back to the parent).
* `translatef/rotatef/scalef` : multiply the top matrix by a translate/rotate/scale matrix (GLM).
* `drawBox(sx,sy,sz)` : `model = scale(topMatrix, (sx,sy,sz))`, then draws the unit cube with that model matrix.

`src/compat/GL/glut.h` maps the old names: `glPushMatrix()` -> `GLCompat::pushMatrix()`,
`glutSolidSphere(r,..)` -> `GLCompat::drawSphere(r,..)`, etc. So the scene code reads like
classic OpenGL, but underneath it is modern shaders + VAO/VBO.

### 3.5 Shader class (src/engine/Shader.cpp)

Reads the two .vert/.frag files, `glCompileShader` each, `glLinkProgram`, then
`SetMat4 / SetVec3 / SetFloat / SetBool` call `glUniform...` to send values.

### 3.6 Texture class (src/engine/Texture.cpp)

`stbi_load` reads the PNG -> `glGenTextures` -> `glTexImage2D` -> `glGenerateMipmap`.
Wrap = `GL_REPEAT`, filter = `GL_LINEAR_MIPMAP_LINEAR` (smooth at any distance).

- - -

## PART 4 — TRANSFORMATIONS (translate, rotate, scale) AND HIERARCHY

### 4.1 What each does

* **Translation** `glTranslatef(x,y,z)` — move.
* **Rotation** `glRotatef(angle, ax, ay, az)` — turn by `angle` degrees around an axis.
* **Scaling** `glScalef(sx,sy,sz)` — stretch/shrink. A cube scaled (0.12, 0.30, 0.12) becomes an arm.

### 4.2 Order matters (very common question!)

Code like this:

```
glTranslatef(...);   // 1st line
glRotatef(...);      // 2nd line
glScalef(...);       // 3rd line
drawCube();
```

makes matrix = T x R x S. A vertex is hit by **S first, then R, then T** (read the code
bottom-to-top for what happens to the shape): scale the cube, rotate it, then move it.
That is why we always write translate first and scale last.

### 4.3 Hierarchy = push/pop

`glPushMatrix()` saves the current coordinate system. Everything drawn after is relative to it.
`glPopMatrix()` restores it. **Anything between push and pop belongs to that parent.**

Real example — the doll's arm (src/doll/DollBody.cpp, `drawArticulatedArm`):

```
glPushMatrix();                              // start arm
  glTranslatef(side*0.44f, 1.72f, 0);        // move to shoulder (relative to body)
  [draw sleeve sphere]
  glRotatef(-46, 1,0,0);                     // swing whole arm forward
  [draw upper arm: translate down, scale to arm size, drawCube]
  glTranslatef(0, -0.30f, 0);                // move down to the elbow  (inherits the swing!)
  [draw elbow ball]
  glRotatef(58, 1,0,0);                      // bend the forearm (inherits upper-arm rotation)
  [draw forearm, hand]
glPopMatrix();                               // back to body
```

**Inheritance:** the forearm never says "rotate -46 degrees" — it inherits it because the matrix
was not popped. If you change the shoulder rotation, the elbow, forearm and hand all follow.

The whole doll tree (slide 11):

```
Doll   : translate(x,y,z) + scale(DOLL_SCALE)        <- Doll.cpp, drawDollOnly()
 |-- Body  : drawBody()                              <- DollBody.cpp
 |     arms (shoulder > upper arm > elbow > forearm > hand), skirt, shirt, legs
 |-- Head  : translate(0, 2.5, 0) then rotate(headAngle)   <- DollHead.cpp
       neck ring, face, eyes, nose, lips, brows, hair, ears, pigtails, beads
```

Scale the doll once (`DOLL_SCALE = 2.6`) -> every part grows. Rotate the head once
(`glRotatef(doll.headAngle, 0,1,0)` at the top of `drawHead`) -> all 51 face parts turn together.

### 4.4 Where each transform is used in the game

* Translation: players walking (`player.z += speed`), jump arc, placing every part.
* Rotation: head turn 180 degrees, arm/leg swing, falling over (`glRotatef(fallAngle...)`), camera orbit.
* Scaling: cube -> torso/arm/leg/glass panel, sphere -> head/hands/eyes, shard boxes.

- - -

## PART 5 — HOW EACH OBJECT IS CREATED FROM SCRATCH

**General recipe (use it for every object):**

1. Decide the pieces (torso = box, head = sphere ...).
2. `glPushMatrix()` — start the object.
3. For each piece: `glPushMatrix()`, `translate` to its place, `rotate` if needed,
`scale`/size it, call the draw function, `glPopMatrix()`.
4. `setMaterial(r,g,b, shininess)` before each piece to give its color.
5. `glPopMatrix()` — end the object.

### 5.1 Player 456 and NPC contestants — src/contestants/Contestants.cpp

Function `drawContestantFigure(legAngle, armAngle, number, isAlive, fallAngle)`.

* 53 parts = **49 boxes + 4 spheres** (see report/source/figures/part\_counts.txt).
* Teal tracksuit torso, arms, legs = scaled **cubes**; white stripes = thin cubes; shoes = cubes;
head and hands = **spheres**; hair = a sphere cap.
* The bib number is drawn with **7-segment digits made of thin boxes**: `drawDigit3D` has a table
`segs[10][7]` saying which of the 7 bars are on for digits 0-9. `draw3DigitNumber` places 3 digits.
* **Walking:** `legAngle = WALK_AMPLITUDE * sin(WALK_OMEGA * walkTime)`, `armAngle = -legAngle`
(arms swing opposite to legs). Each leg/arm is rotated by its angle around the hip/shoulder.
* **Falling when shot:** `glRotatef(fallAngle, 1,0,0.2)`, `fallAngle` rises to 90 degrees.
* Player and NPCs use the same function; the player is just drawn with number 456.

### 5.2 Pink Soldiers (guards) — src/guards/Guards.cpp

* Pink jumpsuit and black mask built from boxes/spheres; the mask symbol (circle / triangle /
square, `enum GuardType`) is drawn by `drawMaskSymbol` . `drawGun` = boxes + cylinder.
* `triggerShot(guards, x, z)`: finds the closest guard (distance squared loop), sets
`isShooting = true` for 0.35 s, and **turns the guard to face the victim**:
`facingAngle = atan2f(dx, dz) * 180 / PI`.

### 5.3 The doll body — src/doll/DollBody.cpp

* Orange pleated skirt: `drawPleatedSkirt()` uses custom triangles (`glBegin(GL_TRIANGLES)`)
around a circle, alternating radius to make pleats.
* Yellow collar shirt (boxes), puffed sleeves (spheres), ball-joint elbows (spheres + thin box),
porcelain arms (scaled boxes), legs, black shoes. Arms are angled to hug the tree.

### 5.4 The big face — src/doll/DollHead.cpp, `drawHead` (51 parts: 40 spheres, 10 boxes, 1 cylinder)

1. **Neck ring:** a dark cylinder (`drawCylinder`).
2. **Cranium (face):** ONE sphere scaled `glScalef(0.44, 0.50, 0.42)` -> an egg-shaped ivory head.
3. **Nose:** a thin tall sphere (bridge) + small sphere (tip) + two tiny spheres (nostrils),
placed at z \~ 0.41 so they sit on the front surface.
4. **Lips:** upper lip = two small flattened spheres (Cupid's bow) + a peak; lower lip = one flattened sphere. Orange-red material, high shininess.
5. **Blush:** two flattened spheres drawn with **alpha 0.32 and blending** (`GL_BLEND`) so they look
see-through.
6. **Eyes** `drawEye`: stacked scaled spheres: white sclera, black iris, tiny pupil, white catchlight
sparkle, plus a thin box eyelash line. During red light a **red glowing sphere** appears in the pupil.
7. **Eyebrows:** two angled thin boxes per brow (an arch).
8. **Hair:** a bigger black sphere dome behind (0.455, 0.495, 0.435), a flat box for the bangs,
a purple box barrette.
9. **Ears:** 4 small scaled spheres each (`drawDollEar`).
10. **Pigtails:** 2 spheres each + purple bead spheres (`drawPigtail`, `drawBeadHairTie`).
11. **Red laser beams:** thin cylinders with **additive blending**, unlit, pulsing:
`pulse = 0.26 + 0.10*sin(scanAlpha*14)`.
12. **Head turn:** top of `drawHead`: `glRotatef(doll.headAngle, 0,1,0)`. All features are drawn
after it, so they all rotate together. Facing direction = local +Z.

### 5.5 The dead tree — src/doll/DollTree.cpp

* `drawBranchSegment(baseR, tipR, len)`: a **cylinder** (tapered) + a **sphere knuckle** at the tip so
branches join with no gap. Bark texture applied here.
* Trunk: 3 segments, each translated up and rotated slightly (so it curves), ending in a "Grand Fork" sphere.
* `drawTwig(...)` is **recursive**: draw a segment, move to its tip, then fork into 2 smaller
twigs (length x 0.70, `depth - 1`), each rotated by an angle that depends on a `seed`
(so every branch bends differently but the tree is the same every run).
* 8 root buttresses spread around the base (`for i<8`, angle = i \* 2PI/8).
* Total = 157 spheres + 156 cylinders.
* Hierarchy again: each branch is drawn inside its parent's transform.

### 5.6 Environment — src/environment/

* Sand floor `drawArenaGround` (textured quad with `sand.png`).
* Painted sky walls + clouds (`drawPaintedSkyWalls`, `drawPaintedCloudCluster`): custom quads/spheres.
* Sky dome, wheat trim, boundary lines, houses (`EnvironmentHouses.cpp`).
* 7-segment countdown clock (`EnvironmentClock.cpp`): same trick as the bib digits — 55 boxes.

### 5.7 Glass bridge — src/glassbridge/

* `GlassBridgeSteps.cpp`, `drawBridge`: steel I-beams (boxes), pink rails (thin boxes), posts,
and **16 pairs of glass panels** (`drawBox(GB_PANEL_WIDTH, thickness, GB_PANEL_LENGTH)` with
translucent alpha); extra thin boxes mark the revealed safe/broken panels.
* `GlassBridgeEnvironment.cpp`: hall walls, floor, ceiling trusses, safety net, concrete texture.
* `GlassBridgeContestants.cpp`: the 16 queued players with 3D bib numbers.

### 5.8 Light fixtures — src/render/LightFixtures.cpp

Physical models of the lights so you can SEE where light comes from: sun orb with rays,
floodlight tower (`drawArenaFloodlight`), chandelier (`drawBridgeChandelier`), follow-spotlight,
wall lamps (`drawWallLamp`), floodlight array. They are drawn **unlit/emissive** so they glow.

### 5.9 Shape View (key I) — "proof that it is all basic shapes"

`GLCompat::drawShape` counts each shape kind and, if `inspect.colorByShape` is on, paints the part
with `shapeColor(kind)`: box = blue, sphere = orange, cylinder = green, etc. `explode` pushes parts
apart from a center point (used for the "pulled apart" pictures). `Showcase.cpp` renders one model
alone for those pictures.

- - -

## PART 6 — SHADING (src/shaders/)

### 6.1 Vertex shader (default.vert) — 3 jobs

```
FragPos = vec3(model * vec4(aPos, 1.0));              // 1. vertex into WORLD space
Normal  = mat3(transpose(inverse(model))) * aNormal;  // 2. fix the normal (see below)
gl_Position = projection * view * vec4(FragPos, 1.0); // 3. world -> camera -> screen
```

* `model` = the stack's top matrix (all the translate/rotate/scale you wrote).
* `view` = camera matrix. `projection` = perspective matrix.
* **Why inverse-transpose for normals?** If you scale non-uniformly (like a stretched cube/arm),
the normal must still stay perpendicular to the surface. Inverse-transpose does this.
* It outputs FragPos, Normal, color and UV. The **rasterizer interpolates** them across each triangle.

### 6.2 Fragment shader (default.frag) — runs once per PIXEL = Phong shading

**Gouraud vs Phong:** Gouraud lights only the vertices and blends colors; **Phong interpolates the**
**normal and lights EVERY pixel** (smoother highlights). Mine is Phong.

Steps in `main()`:

1. `baseColor = materialColor * VertexColor`; if `useTexture`, multiply by `texture(diffuseTexture, TexCoord * texRepeat)`.
2. If `unlit` (glowing things) -> output baseColor + emissive without lighting.
3. `N = normalize(Normal)` (re-normalize because interpolation shortens it) and `V = normalize(viewPos - FragPos)`.
4. **Ambient** (hemisphere): `mix(ambientGround, ambientSky, N.y*0.5+0.5)` — surfaces facing up get sky
color, facing down get ground color.
5. For each light, `addLight(...)`:
    * **Diffuse (Lambert):** `diff = max(dot(N, L), 0)` — brighter when the surface faces the light.
    * **Specular (Blinn-Phong):** `H = normalize(L + V)`, `spec = pow(max(dot(N,H),0), shininess)` — the shiny spot.
6. Final: `litColor = diffuse*baseColor + specular + rim + emissive`.
7. `toneMap` (filmic curve, keeps bright neon from clipping) and `applyFog` (far things fade to sky color).

**Formula to write on the board:**
`I = Ia*ka + sum_over_lights [ Il * ( kd*max(N.L,0) + ks*max(N.H,0)^n ) ]`

### 6.3 Material (how an object gets its look)

`setMaterial(r,g,b, shininess, specularStrength)` sets `materialColor`, `shininess` (n in the
formula; bigger = tighter highlight), `materialSpecularStrength` (ks).
`setEmissive` = the part glows by itself (neon rails, red eyes). `setAlpha` = translucent (glass).

- - -

## PART 7 — LIGHTING (3 types)

Data structs are in **src/core/Types.h**: `DirLightData`, `PointLightData`, `SpotLightData`,
bundled in `SceneLighting`. `GLCompat::beginFrame` (src/engine/GLCompat.cpp \~line 137) copies
them into shader uniforms every frame. The values are set each frame in main\_modern.cpp
(search `DYNAMIC 3-TYPE LIGHTING`, two blocks: one for each round).

| Type | What it is | Round 1 source | Round 2 source | Key | Math |
| ---- | ---------- | -------------- | -------------- | --- | ---- |
| **Directional** | parallel rays, no position, no falloff | Sun | Stage floodlight array | 0 | `L = normalize(-direction)` |
| **Point** | light bulb, all directions, fades with distance | Floodlight tower (18,16,15) | Chandelier + 4 wall lamps | 8 (7 for wall lamps) | `atten = intensity / (c + l*d + q*d*d)` |
| **Spot** | point light inside a cone with soft edge | Doll's eye searchlight (red/green) | Follow-spot on jumper | 9 | `cone = clamp((theta - outerCut) / (innerCut - outerCut), 0, 1)` |

* Spot cone numbers: `cutOff = cos(15 deg) = 0.965`, `outerCutOff = cos(23 deg) = 0.92`.
Between them the edge fades smoothly (soft edge).
* **Doll searchlight follows her head:** `spot.direction = (sin(headAngle), -0.12, cos(headAngle))`;
red color + intensity 2.8 during Red Light, soft green 1.2 during Green Light.
* **Sun direction** comes from two sliders (azimuth, elevation): `LightFixtures::updateSunDirection`.
* Toggle keys flip `sceneLighting.dir.enabled`, `.point.enabled`, `.spot.enabled` (see `keyCallback`).
* Up to 5 point lights (`MAX_POINT_LIGHTS 5` in the shader) = 1 main + 4 wall lamps.
* Fog: `fogDensity = 0.0045`, `visibility = exp(-d*d)`.

### 7.1 The "Sun Sky Position" panel (right ImGui panel, Lighting card)

This small panel controls the **directional light** (the sun in Round 1, the stage floodlight array in Round 2).
It is made with `ImGui::SliderFloat` and `ImGui::Checkbox` in main_modern.cpp (search `Sun Sky Position`, `Azimuth##Sun`).

| Item on the panel | What it does | Variable |
|---|---|---|
| **Azimuth** slider (0-360 deg) | Compass direction of the sun around the arena (turn it left/right around the Y axis). | `sunAzimuth` (starts at 45) |
| **Elevation** slider (10-85 deg) | Height of the sun above the horizon. 10 = low sunset, 85 = almost straight overhead. | `sunElevation` (starts at 38, resets to 50 in some modes) |
| **Sun Rays & Cone** checkbox | Shows or hides extra see-through cone/ray shapes drawn around the sun orb so you can SEE the light beam. It changes only the picture of the beam, not the lighting itself. | `showSunRays` (off by default) |
| **Dir: [-0.45, -0.77, -0.45]** (green text) | The final light direction vector (x, y, z) that is sent to the shader. It always has length 1. | `sceneLighting.dir.direction` |

**How the two angles become the direction (function `LightFixtures::updateSunDirection`, src/render/LightFixtures.cpp):**
```
y = sin(elevation)
x = cos(elevation) * sin(azimuth)
z = cos(elevation) * cos(azimuth)
dir.direction = - normalize(x, y, z)       // minus sign: light travels FROM the sun TO the ground
```
Check with the numbers in the panel (azimuth 45, elevation 50):
y = sin 50 = 0.77, x = cos 50 * sin 45 = 0.64 * 0.71 = 0.45, z = 0.45.
Direction = -(0.45, 0.77, 0.45) = **(-0.45, -0.77, -0.45)**, exactly what the panel shows.
Negative y means the light points **downward**; negative x and z mean it points toward the arena from the +x/+z sky.

**What changes on screen when you drag the sliders**
- The 3D sun orb moves: `drawDirectionalSun` puts it at `55 * (x, y, z)` in the sky (look up and right to see it).
- Every surface is re-lit because the fragment shader uses `L = normalize(-dirLightDir)` and `diffuse = max(dot(N, L), 0)`:
  surfaces facing the sun get brighter, others darker.
- Specular highlights move, and shading on the doll and players changes direction.
- Lower elevation = longer, softer, more sideways light; high elevation = bright top-down light.

**Say in the viva:** "A directional light has no position, only a direction. I let the user choose that
direction with two intuitive angles, azimuth and elevation (spherical coordinates), and convert them to a unit
vector with sin and cos every frame. The vector is uploaded as the uniform `dirLightDir` and used in the Lambert
and Blinn-Phong equations."

**Possible question: why 2 angles and not x, y, z sliders?** Two angles always give a valid unit direction;
three independent numbers would need normalizing and could be zero.

- - -

## PART 8 — TEXTURES

3 images in `assets/textures/` (512x512, seamless/tileable): **sand.png, bark.png, concrete.png**.
They were generated by `tools/make_textures.py` (noise made with FFT so edges tile perfectly).

**Pipeline:**

1. Load once: `SceneTextures::loadAll()` -> `Texture::Load` (stb\_image -> GL texture).
2. Every vertex already has **UV coordinates** (see `Vertex` / `Primitives.cpp`).
3. Before drawing: `GLCompat::setTexture(&SceneTextures::sand(), uRepeat, vRepeat)`.
4. `GLCompat` binds it and sets `useTexture = true` and `texRepeat`.
5. Fragment shader: `baseColor *= texture(diffuseTexture, TexCoord * texRepeat).rgb`.
Multiplying UV by `texRepeat` tiles the image many times; `GL_REPEAT` makes it wrap.
6. `clearTexture()` goes back to plain colors.

| Texture | Used on | Where | Repeat |
| ------- | ------- | ----- | ------ |
| sand | arena floor | Environment.cpp `drawArenaGround` (`setTexture(&SceneTextures::sand())`) | tiled across ground |
| bark | tree trunk, branches | DollTree.cpp `drawBranchSegment` | `around = round(baseR*4)`, along = `len/1.6` so thick/long branches keep the same bark size |
| concrete | bridge hall walls, floor, platforms | GlassBridgeEnvironment.cpp | `size / CONCRETE_TILE` |

- - -

## PART 9 — CAMERA

### 9.1 Matrices (src/engine/Camera.cpp)

```
view       = glm::lookAt(eye, target, up)                       // where the camera is and looks
projection = glm::perspective(radians(fov), width/height, near, far)   // lens: fov, aspect, clip planes
```

`camera.Apply(shader)` uploads `view`, `projection`, `viewPos` (camera position, used for specular).
In the vertex shader: `gl_Position = projection * view * model * position`.

### 9.2 Round 1 camera — src/camera/Camera.cpp (class CameraManager)

7 modes (`enum CameraMode` in Types.h): 1 third-person, 2 free orbit, 3 top-down, 4 doll orbit,
5 left broadcast, 6 right broadcast, 7 finish line.

* `update()` computes a **target eye** and **target look point** for the current mode.
Third person: `eye = player - (sin(yaw), cos(yaw)) * distance`, height + pitch, clamped inside the arena.
* **Smoothing (exponential damping):**
`factor = 1 - exp(-speed * dt); smoothCam += (targetEye - smoothCam) * factor;`
Frame-rate independent, makes switching modes glide.
* `getViewVectors` returns eye/look/up/fov; main loop calls `camera.SetView(...)`.
* Top-down mode is fixed: `gluLookAt(0,110,-3.5, 0,0,-3.5, 0,0,1)`.

### 9.3 Mouse control

`handleMouseMotion`: dragging changes `camYaw += dx*0.35` and `camPitch -= dy*0.25`
(pitch clamped -20..80). Wheel -> `zoom()` changes distance (clamped 1.8..18).

### 9.4 Round 2 — src/glassbridge/GlassBridgeCamera.cpp (`getCameraVectors`)

5 modes each with hand-placed eye/look/fov (52-55 deg), smoothed in main\_modern.cpp with the same
exponential formula (speed 9.5 or 7.5).

### 9.5 CCTV doll-cam (Pass 3)

Second render into a small rectangle: `glScissor`, `glViewport`, `getDollCloseupVectors`,
`camera.SetView(...)`, draw ground + doll + guards only. Toggle with F4.

- - -

## PART 10 — MOVEMENT, ANIMATION AND GAME LOGIC

### 10.1 Player movement (main\_modern.cpp, main loop)

Keys set booleans in `keyCallback` (`keyForward = isPress` ...). Each frame:

```
player.facingAngle += TURN_SPEED * dtScale   (A/D)
forwardX = sin(rad); forwardZ = cos(rad)
player.x += forwardX * speed * dtScale        (W)
```

`rad` = facing angle in radians. `dtScale = dt*60` keeps speed the same at any FPS.
Speed tiers (Config.h): walk 0.038, jog 0.080, sprint 0.155 (keys Z, X, C).
Walk cycle: `walkTime += dt`, `legAngle = 28*sin(10*walkTime)`.

### 10.2 Doll state machine (src/doll/Doll.cpp, `DollManager::update`)

Two states: `GREEN_LIGHT` and `RED_LIGHT`, with a countdown `stateTimer`.

* GREEN: `targetAngle = 0` (back to the tree), song plays, timer 4.8-6.2 s.
* When the timer ends -> RED: `targetAngle = 180`, stop song, play click, timer 3.2-5.2 s.
* Head eases to the target: `headAngle += (targetAngle - headAngle) * 15 * dt` (fast at first, slows down).

### 10.3 Elimination

* Player: `checkPlayerElimination` — if `lightState == RED_LIGHT && headAngle > 80 && isMoving` -> CAUGHT:
guard shoots (`triggerShot`), blood particles, game-over music. Reaching `FINISH_Z (45)` -> WON.
* NPCs (`ContestantsManager::update`): green = walk with their own random speed
(`0.040 + rand()%22/1000`), red = freeze; once `headAngle > 100` each NPC has an **18% chance** to die
(`rand() % 100 < 18`) -> guard shoots, blood, they fall (`fallAngle` += 280\*dt up to 90).
* The count shown (456) is scaled: `living/32 * 455 + player`.

### 10.4 Particles (blood and glass)

A particle is a struct with position, velocity, life. Each frame:
`pos += vel*dt; vel.y -= 9.8*dt (gravity); life += dt; alpha = 1 - life/maxLife;` and delete when life ends.
Blood: 30 spheres (`spawnBloodParticles`). Drawn with alpha blending, depth-write off.

### 10.5 Glass bridge logic (src/glassbridge/GlassBridge.cpp)

* `init`: 16 steps; for each `safeSide = (rand()%2==0) ? LEFT : RIGHT` (50/50). Tempered = safe, normal = breaks.
* 16 contestants in a queue; one active at a time (`activeQueueIndex`). Player is at a random queue number.
* AI jump: wait `aiThinkTimer`, pick random side, jump.
* **Jump arc:** `jumpProgress` 0->1 at 2.4/s. x,z interpolate linearly between start and target;
`y = BRIDGE_HEIGHT + sin(p * PI) * 1.30` -> a smooth parabola-like hop. Legs: `25*sin(2*PI*p)`.
* On landing: if `landedSide == step.safeSide` -> safe, step sound; else **break**: play shatter,
`spawnShards`, contestant starts falling (`fallVelocity += 32*dt`, `y -= fallVelocity*dt`).
* Player jump: `jumpPlayer(...)` sets target panel X (`GB_LEFT_PANEL_X` or right) and `targetZ = start + step*length`.
* `skipToPlayerTurn` (key S) simulates all earlier contestants instantly; each failure reveals which
panel was safe, so the player benefits.

### 10.6 Shards (GlassBridgeShards.cpp)

`spawnShards` creates 55 shards with random velocity (up 2-6.5, sideways +-3.75), random rotation and
spin speed (+-300 deg/s), random size 0.08-0.24, light-blue color. In `update`:
`vy -= 28*dt` (gravity), position += velocity, rotation += spin, `life -= dt`, `alpha = life/2`.
Drawn as small boxes (`drawBox(size, size*0.35, size*1.5)`) with blend + emissive.

- - -

## PART 11 — USER INTERACTION

**Keyboard** — `keyCallback` (main\_modern.cpp \~line 309), GLFW key callback (called on press/release).
Movement keys are held states (`keyForward`...), toggles are done on press.

| Key | Action |
| --- | ------ |
| W/S, A/D (or arrows) | walk / turn (Round 1); A/D = jump left/right panel (Round 2) |
| Z / X / C | walk / jog / sprint |
| 1-7 | Round 1 cameras; 1-5 in Round 2 |
| 0, 8, 9, 7 | directional / point / spot / wall lamps |
| I | Shape View |
| P | switch round; R restart |
| Space/Enter | start Round 1 |
| Tab / S (Round 2) | fast-forward AI / skip to player's turn |
| F3 / F4 | VSync / CCTV picture-in-picture |
| Esc | exit |

**Mouse** — `mouseButtonCallback` (press = start drag), `cursorPosCallback` (drag = orbit),
`scrollCallback` (wheel = zoom). They first check `io.WantCaptureMouse` so clicking an ImGui
panel does not also rotate the camera.

**ImGui panels** — Dear ImGui is an immediate-mode GUI. Each frame the code calls
`ImGui::Begin("LeftPanel")`, `ImGui::Checkbox(...)`, `ImGui::SliderFloat("Azimuth", &sunAzimuth, 0, 360)`,
then `ImGui::Render()`. Left = timer, players alive, progress, radar, CCTV. Right = camera buttons,
controls help, VSync, lighting card (checkboxes for each light + sun sliders).

- - -

## PART 12 — AUDIO AND EXTRAS

* **Audio** (src/audio/Audio.cpp): beeps, boom, glass shatter, gunshot, clicks are **generated in code**
as waveforms using `sin` (with an envelope so there are no clicks) into WAV buffers; the singing
uses `squid_song.mp3` via Windows MCI. Song stops hard when red light starts.
* **Capture mode** `game_modern.exe --capture folder` plays scripted shots and saves BMPs
(used for the report and slides).
* **tools/make\_textures.py** generates the textures; **make\_presentation.py** builds the slides.

- - -

## PART 13 — LIKELY VIVA QUESTIONS WITH ANSWERS

1. **What is OpenGL and which version?** Graphics API; I use 3.3 Core profile (no old fixed pipeline), with GLFW (window/input), GLAD (function loader), GLM (math), stb\_image (PNG), Dear ImGui (UI).
2. **Fixed pipeline vs programmable?** Fixed = built-in lighting. Programmable = I write the vertex and fragment shaders myself (default.vert / default.frag).
3. **Explain the rendering pipeline.** Vertices -> vertex shader (model/view/projection) -> rasterization (triangles to fragments, interpolation) -> fragment shader (lighting, texture) -> depth test -> framebuffer.
4. **What are VAO, VBO, EBO?** See 3.2.
5. **How do you make a human with only boxes and spheres?** Scaled cubes for torso/limbs, spheres for head/hands, joined by push/pop hierarchy. See 5.1.
6. **What do glPushMatrix/PopMatrix do?** Save/restore the transform; creates parent-child hierarchy. See 4.3.
7. **Order of transformations?** T x R x S; vertex gets scaled first. See 4.2.
8. **Why is a unit mesh scaled instead of making a new mesh per size?** Saves memory; only 7 meshes on GPU; scale is just a matrix.
9. **What is a normal and why do we need it?** Direction perpendicular to the surface; needed for N.L diffuse and specular.
10. **Phong vs Gouraud?** Gouraud: light at vertices then interpolate color. Phong: interpolate normal, light every pixel. Mine = Phong (lighting in the fragment shader).
11. **Phong vs Blinn-Phong?** Phong specular uses reflection vector R.V; Blinn uses halfway vector H=normalize(L+V), N.H — cheaper and nicer. I use Blinn-Phong specular.
12. **What are ambient, diffuse, specular?** Constant base light / Lambert angle-based / shiny highlight.
13. **Difference between the 3 lights?** Table in Part 7.
14. **What is attenuation?** Light getting weaker with distance: 1/(c + l*d + q*d^2).
15. **How does the spotlight cone work?** Compare angle theta (dot of directions) to cutOff cos(15) and outerCutOff cos(23); between them fade smoothly.
16. **What is a texture / UV?** Image mapped on a surface; UV tell each vertex which point of the image it has. `texRepeat` tiles it.
17. **Why GL\_REPEAT and mipmaps?** Repeat tiles image; mipmaps keep distant textures smooth (no shimmer).
18. **What is the model/view/projection matrix?** Object->world / world->camera / camera->screen (perspective).
19. **What does lookAt do?** Builds the view matrix from eye, target, up.
20. **What is FOV?** Field of view angle of the lens; 50-55 degrees in my cameras.
21. **Why is dt used?** Frame-rate independent movement.
22. **What is the depth buffer?** Stores nearest depth per pixel so closer objects cover farther ones (`GL_DEPTH_TEST`).
23. **How does transparency (glass, blush) work?** `GL_BLEND` with `alpha` (SRC\_ALPHA, 1-SRC\_ALPHA); depth writing turned off for transparent things so they do not block each other.
24. **What is the emissive material?** A part that glows by itself (neon, red eyes); added after lighting.
25. **How is the head turn made?** One `glRotatef(headAngle, 0,1,0)` before drawing all head parts; headAngle eased toward 0 or 180.
26. **How did you do the tree?** Recursion: each branch = cylinder + knuckle sphere; recursive twig forks into two smaller rotated twigs.
27. **How does collision/elimination work?** No physics engine; simple rules on `headAngle`, light state and `rand()`.
28. **What is exponential smoothing?** `x += (target - x) * (1 - exp(-k*dt))`.
29. **What does the `unlit` flag do?** Skips lighting; flat color (light halos, beams).
30. **What is fog?** Mix with sky color by distance: `exp(-d*d)`.
31. **What does ImGui do and how is it different?** Immediate-mode UI: you describe the UI every frame in code, no stored widget objects.
32. **What would you improve?** Shadow mapping, more textures + normal maps, imported meshes, skeletal animation, more rounds, post-processing (bloom).

- - -

## PART 14 — "CHANGE X LIVE" CHEAT SHEET (teacher says: change this now)

All in `src/core/Config.h` unless noted. After editing, rebuild (see README "Build & Run").

| Change | Edit |
| ------ | ---- |
| Round 1 time 60s -> 90s | `GAME_TIME_LIMIT = 90.0f;` |
| Player speed | `SPEED_SLOW_WALK`, `SPEED_EASY_RUN`, `SPEED_FAST_SPRINT` |
| Doll size | `DOLL_SCALE` |
| Number of NPCs | `NUM_NPC_CONTESTANTS` |
| Tracksuit / guard / sand colors | `COLOR_TRACKSUIT`, `COLOR_GUARD_MAGENTA`, `COLOR_ARENA_SAND` |
| Glass bridge steps | `GB_NUM_STEPS` |
| NPC death chance (18%) | Contestants.cpp: `(rand() % 100) < 18` |
| How fast the head turns | Doll.cpp: `diff * 15.0f * dt` |
| How long red light lasts | Doll.cpp: `doll.stateTimer = 3.2f + ...` |
| Spotlight cone width | Types.h `cutOff`, `outerCutOff` (cos of angle) |
| Light colors / intensity | main\_modern.cpp lighting block, e.g. `sceneLighting.point.intensity` |
| Point-light fall-off | Types.h `constant`, `linear`, `quadratic` |
| Shininess of an object | its `setMaterial(r,g,b, SHININESS, spec)` call |
| Texture tiling | the `setTexture(tex, uRep, vRep)` call |
| Remove a texture from something | delete/replace its `setTexture` call (or `clearTexture()`) |
| Camera field of view | main\_modern.cpp `camera.SetView(..., fov, ...)` or `fov = 55.0f` in GlassBridgeCamera.cpp |
| Camera smoothing speed | Camera.cpp `interpSpeed` (12 / 8) |
| Make a new object | copy the pattern in 5.x: push, translate, setMaterial, drawBox/Sphere, pop |
| Add a new key | `keyCallback` in main\_modern.cpp: `case GLFW_KEY_X:` |

### How to make a NEW simple object (e.g. a table) — write this on paper if asked

``` cpp
void drawTable() {
    glPushMatrix();
        glTranslatef(x, 0, z);                       // place the table
        Renderer::setMaterial(0.5f, 0.3f, 0.1f, 20); // wood color
        glPushMatrix();                              // table top
            glTranslatef(0, 0.8f, 0);
            Renderer::drawBox(2.0f, 0.1f, 1.0f);     // width, height, depth
        glPopMatrix();
        for (int i = 0; i < 4; ++i) {                // 4 legs
            glPushMatrix();
                glTranslatef((i%2 ? 0.9f : -0.9f), 0.4f, (i/2 ? 0.4f : -0.4f));
                Renderer::drawBox(0.1f, 0.8f, 0.1f);
            glPopMatrix();
        }
    glPopMatrix();
}
```

Then call `drawTable();` inside the draw section of the main loop (next to `DollManager::draw(doll);`).

- - -

## PART 15 — GLOSSARY (one line each)

* **Vertex** — a point with position, normal, color, UV.
* **Mesh** — vertices + indices drawn as triangles.
* **Shader** — small GPU program. Vertex shader = per vertex, fragment shader = per pixel.
* **Uniform** — a value the CPU sends to the shader (matrices, light data).
* **Normal** — unit vector perpendicular to a surface.
* **Fragment** — one candidate pixel produced by rasterization.
* **Matrix stack** — saved list of transforms; push/pop gives hierarchy.
* **Model/View/Projection** — object->world, world->camera, camera->screen.
* **UV** — 2D texture coordinates (0..1).
* **Mipmap** — pre-shrunk copies of a texture for distant surfaces.
* **Attenuation** — light loss over distance.
* **Alpha / blending** — transparency.
* **Emissive** — self-lit color.
* **Delta time (dt)** — seconds since the last frame.
* **State machine** — object with a few states and rules to change between them (the doll: GREEN <-> RED).
* **Hierarchy** — child transforms are relative to the parent's.