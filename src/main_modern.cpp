#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Modern engine headers
#include "engine/Shader.h"
#include "engine/Camera.h"
#include "engine/GLCompat.h"

// Game subsystems
#include "core/Config.h"
#include "core/Types.h"
#include "audio/Audio.h"
#include "render/Renderer.h"
#include "doll/Doll.h"
#include "guards/Guards.h"
#include "contestants/Contestants.h"
#include "environment/Environment.h"
#include "camera/Camera.h"
#include "glassbridge/GlassBridge.h"

// Dear ImGui modern UI
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace {
    // Current Active Game Phase
    GamePhase currentPhase = PHASE_RED_LIGHT_GREEN_LIGHT;

    // -----------------------------------------------------------------------------------
    // ROUND 1: RED LIGHT, GREEN LIGHT STATE
    // -----------------------------------------------------------------------------------
    Player player;
    Doll doll;
    std::vector<Guard> guards;
    std::vector<NPCContestant> npcs;
    std::vector<Particle> particles;

    GameState gameState = WAITING;
    CameraMode camMode = CAM_THIRD_PERSON;

    float gameTimer = GAME_TIME_LIMIT;
    int aliveCount = TOTAL_CONTESTANTS;

    // -----------------------------------------------------------------------------------
    // ROUND 2: SQUID GAME GLASS BRIDGE STATE
    // -----------------------------------------------------------------------------------
    std::vector<GlassStep> gbSteps;
    std::vector<GBContestant> gbContestants;
    std::vector<GlassShard> gbShards;
    int gbPlayerQueueNum = 1;
    int gbActiveQueueIndex = 0;
    GlassBridgeState gbState = GB_PLAYING;
    GlassBridgeCamMode gbCamMode = GB_CAM_FOLLOW;
    float gbTimer = GB_TIME_LIMIT;
    float gbOrbitYaw = 0.0f;
    float gbOrbitPitch = 18.0f;
    float gbOrbitDist = 6.8f;
    bool gbFastForward = false;

    // -----------------------------------------------------------------------------------
    // WINDOW & INPUT STATE
    // -----------------------------------------------------------------------------------
    int windowWidth = 1100;
    int windowHeight = 720;

    bool keyForward = false;
    bool keyBackward = false;
    bool keyTurnLeft = false;
    bool keyTurnRight = false;
    bool keyStrafeLeft = false;
    bool keyStrafeRight = false;
    bool keyHoldSprint = false;
    bool keyHoldWalk   = false;

    bool isMouseDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    // Glass Bridge Smooth Camera Tracking
    glm::vec3 gbSmoothEye(0.0f, GB_BRIDGE_HEIGHT + 3.0f, -4.0f);
    glm::vec3 gbSmoothLook(0.0f, GB_BRIDGE_HEIGHT + 1.1f, 10.0f);
    bool gbCamInitialized = false;

    // -----------------------------------------------------------------------------------
    // 3-TYPE LIGHTING & SHADING LAB STATE & FIXTURES
    // -----------------------------------------------------------------------------------
    SceneLighting sceneLighting;

    void drawArenaFloodlight(const PointLightData& pt) {
        glPushMatrix();
        glTranslatef(pt.position.x, 0.0f, pt.position.z);

        // 1. Tall Steel Lattice/Pylon Pole from ground y=0 to y=pt.position.y
        GLCompat::setMaterial(0.18f, 0.20f, 0.24f, 40.0f);
        glPushMatrix();
        glTranslatef(0.0f, pt.position.y * 0.5f, 0.0f);
        GLCompat::drawBox(0.45f, pt.position.y, 0.45f);
        glPopMatrix();

        // 2. Mounting crossbar at top
        glPushMatrix();
        glTranslatef(0.0f, pt.position.y, 0.0f);
        GLCompat::drawBox(2.4f, 0.25f, 0.6f);

        // 3. Floodlight reflector housing
        GLCompat::setMaterial(0.12f, 0.12f, 0.14f, 60.0f);
        glPushMatrix();
        glTranslatef(0.0f, -0.2f, 0.0f);
        GLCompat::drawBox(2.0f, 0.6f, 0.5f);
        glPopMatrix();

        // 4. Glowing Lamp Face (Warm tungsten/golden if ON, dark if OFF)
        if (pt.enabled) {
            GLCompat::setMaterial(1.0f, 0.95f, 0.80f, 100.0f);
            GLCompat::setEmissive(pt.color.r, pt.color.g, pt.color.b, 2.0f);
        } else {
            GLCompat::setMaterial(0.2f, 0.2f, 0.2f, 10.0f);
            GLCompat::resetEmission();
        }
        glPushMatrix();
        glTranslatef(0.0f, -0.25f, -0.26f);
        GLCompat::drawBox(1.8f, 0.45f, 0.08f);
        glPopMatrix();
        GLCompat::resetEmission();

        glPopMatrix();
        glPopMatrix();
    }

    // -----------------------------------------------------------------------------------
    // DIRECTIONAL LIGHT SUN VISUALS (Visible glowing Sun disc & stage floodlight array)
    // -----------------------------------------------------------------------------------
    float sunAzimuth = 45.0f;    // degrees around Y (Upper-Right Sky)
    float sunElevation = 38.0f;  // degrees above ground (directly visible above arena wall)
    bool  showSunRays = true;

    void updateSunDirection() {
        float elRad = glm::radians(sunElevation);
        float azRad = glm::radians(sunAzimuth);
        // Position on sky dome
        float y = sinf(elRad);
        float x = cosf(elRad) * sinf(azRad);
        float z = cosf(elRad) * cosf(azRad);
        // Direction from sun pointing down to arena center
        sceneLighting.dir.direction = -glm::normalize(glm::vec3(x, y, z));
    }

    void drawBridgeChandelier(const PointLightData& pt) {
        glPushMatrix();
        glTranslatef(pt.position.x, pt.position.y, pt.position.z);

        // 4 Rigid suspension rigging cables extending up to structural ceiling trusses (Y=28.5)
        GLCompat::setMaterial(0.22f, 0.24f, 0.26f, 40.0f);
        for (int c = 0; c < 4; ++c) {
            float ca = (float)c * 1.570796f; // 90 deg intervals
            float cx = cosf(ca) * 1.5f;
            float cz = sinf(ca) * 1.5f;
            glPushMatrix();
            glTranslatef(cx, 2.75f, cz);
            GLCompat::drawBox(0.06f, 5.5f, 0.06f);
            glPopMatrix();
        }

        // Heavy industrial dual concentric octagonal chandelier rings
        GLCompat::setMaterial(0.28f, 0.25f, 0.20f, 60.0f); // Aged bronze/dark brass
        GLCompat::drawTorus(0.14f, 2.2f, 12, 24);
        GLCompat::drawTorus(0.12f, 1.3f, 10, 20);

        // 8 Perimeter warm bulb pendants around outer ring
        for (int i = 0; i < 8; ++i) {
            float ang = (float)i * 0.785398f; // 45 deg intervals
            float bx = cosf(ang) * 2.2f;
            float bz = sinf(ang) * 2.2f;
            glPushMatrix();
            glTranslatef(bx, -0.22f, bz);
            if (pt.enabled) {
                GLCompat::setMaterial(1.0f, 0.92f, 0.70f, 100.0f);
                GLCompat::setEmissive(pt.color.r * 1.2f, pt.color.g * 1.2f, pt.color.b * 1.2f, 2.0f);
            } else {
                GLCompat::setMaterial(0.2f, 0.2f, 0.2f, 10.0f);
                GLCompat::resetEmission();
            }
            GLCompat::drawSphere(0.25f, 10, 10);
            glPopMatrix();
        }

        // Central Colossal Luminous Core (Point Light Source)
        if (pt.enabled) {
            GLCompat::setMaterial(1.0f, 0.96f, 0.82f, 100.0f);
            GLCompat::setEmissive(pt.color.r * 1.8f, pt.color.g * 1.6f, pt.color.b * 1.3f, 3.5f);
            GLCompat::drawSphere(0.95f, 20, 20);

            // Radiant Omnidirectional Amber Glow Halo
            GLCompat::setUnlit(true);
            GLCompat::enableBlend();
            GLCompat::setDepthWrite(false);
            GLCompat::setAlpha(0.26f);
            GLCompat::setMaterial(1.0f, 0.85f, 0.45f, 10.0f);
            GLCompat::drawSphere(2.8f, 16, 16);
            GLCompat::setAlpha(0.10f);
            GLCompat::drawSphere(5.2f, 16, 16);
            GLCompat::disableBlend();
            GLCompat::setDepthWrite(true);
            GLCompat::setUnlit(false);
            GLCompat::setAlpha(1.0f);
            GLCompat::resetEmission();
        } else {
            GLCompat::setMaterial(0.15f, 0.15f, 0.18f, 10.0f);
            GLCompat::resetEmission();
            GLCompat::drawSphere(0.95f, 16, 16);
        }

        glPopMatrix();
    }

    void drawFollowSpotlight(const SpotLightData& sp) {
        glPushMatrix();
        glTranslatef(sp.position.x, sp.position.y, sp.position.z);

        // Theater spotlight cylindrical projector housing
        GLCompat::setMaterial(0.18f, 0.20f, 0.24f, 60.0f);
        glPushMatrix();
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GLCompat::drawCylinder(0.40f, 0.52f, 1.2f, 16);
        glPopMatrix();

        // Glowing projector lens
        if (sp.enabled) {
            GLCompat::setMaterial(0.95f, 0.98f, 1.0f, 100.0f);
            GLCompat::setEmissive(sp.color.r * 1.8f, sp.color.g * 1.8f, sp.color.b * 1.8f, 3.0f);
            glPushMatrix();
            glTranslatef(0.0f, -1.22f, 0.0f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            GLCompat::drawDisk(0.0f, 0.50f, 16);
            glPopMatrix();

            // Downward Tracking Volumetric Spotlight Cone onto active contestant
            if (showSunRays) {
                GLCompat::setUnlit(true);
                GLCompat::enableBlend();
                GLCompat::setDepthWrite(false);
                GLCompat::setAlpha(0.14f);
                GLCompat::setMaterial(sp.color.r, sp.color.g, sp.color.b, 10.0f);
                glPushMatrix();
                glTranslatef(0.0f, -4.2f, 0.0f);
                GLCompat::drawCylinder(0.48f, 3.0f, 7.8f, 16);
                glPopMatrix();
                GLCompat::disableBlend();
                GLCompat::setDepthWrite(true);
                GLCompat::setUnlit(false);
                GLCompat::setAlpha(1.0f);
            }
            GLCompat::resetEmission();
        } else {
            GLCompat::setMaterial(0.2f, 0.2f, 0.2f, 10.0f);
            GLCompat::resetEmission();
            glPushMatrix();
            glTranslatef(0.0f, -1.22f, 0.0f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            GLCompat::drawDisk(0.0f, 0.50f, 16);
            glPopMatrix();
        }

        glPopMatrix();
    }

    void drawSun(const DirLightData& dir) {
        // Place sun high in the sky in the exact opposite direction of sunlight travel
        glm::vec3 sunDir = -glm::normalize(dir.direction);
        float sunDist = 65.0f; // High above arena walls (WALL_HEIGHT = 22.0f)
        glm::vec3 sunPos = sunDir * sunDist;

        glPushMatrix();
        glTranslatef(sunPos.x, sunPos.y, sunPos.z);

        if (dir.enabled) {
            // 1. Brilliant Luminous Solar Core
            GLCompat::setMaterial(1.0f, 0.98f, 0.90f, 100.0f);
            GLCompat::setEmissive(1.8f, 1.60f, 1.15f, 3.5f);
            GLCompat::drawSphere(4.2f, 22, 22);

            // 2. Radiant Atmospheric Solar Corona / Halos
            GLCompat::setUnlit(true);
            GLCompat::enableBlend();
            GLCompat::setDepthWrite(false);

            GLCompat::setAlpha(0.40f);
            GLCompat::setMaterial(1.0f, 0.88f, 0.40f, 10.0f);
            GLCompat::drawSphere(7.2f, 18, 18);

            GLCompat::setAlpha(0.20f);
            GLCompat::setMaterial(1.0f, 0.72f, 0.25f, 10.0f);
            GLCompat::drawSphere(11.5f, 18, 18);

            // 3. Radiant Solar Flare Rays / Spikes
            if (showSunRays) {
                GLCompat::setAlpha(0.30f);
                GLCompat::setMaterial(1.0f, 0.90f, 0.50f, 5.0f);
                for (int ray = 0; ray < 8; ++ray) {
                    float rayAngle = ray * 45.0f;
                    glPushMatrix();
                    glRotatef(rayAngle, sunDir.x, sunDir.y, sunDir.z);
                    glRotatef(rayAngle * 0.5f, 0.0f, 1.0f, 0.0f);
                    GLCompat::drawBox(0.35f, 18.0f, 0.35f);
                    glPopMatrix();
                }

                // 4. Directional Sunbeam Shaft pointing down into arena
                GLCompat::setAlpha(0.12f);
                GLCompat::setMaterial(1.0f, 0.95f, 0.70f, 5.0f);
                glPushMatrix();
                // Draw a subtle translucent shaft along light direction towards arena
                glm::vec3 shaftVec = dir.direction * 35.0f;
                glTranslatef(shaftVec.x * 0.5f, shaftVec.y * 0.5f, shaftVec.z * 0.5f);
                GLCompat::drawBox(6.0f, 35.0f, 6.0f);
                glPopMatrix();
            }

            GLCompat::disableBlend();
            GLCompat::setDepthWrite(true);
            GLCompat::setUnlit(false);
            GLCompat::setAlpha(1.0f);
            GLCompat::resetEmission();
        } else {
            // Eclipsed / Dimmed Sun when Directional Light is toggled OFF (Key 0)
            GLCompat::setMaterial(0.12f, 0.12f, 0.15f, 15.0f);
            GLCompat::resetEmission();
            GLCompat::drawSphere(4.2f, 16, 16);
        }

        glPopMatrix();
    }

    void drawStageSunBank(const DirLightData& dir) {
        // Place overhead stage light array on ceiling trusses directly above bridge
        glm::vec3 bridgeCenter(0.0f, GB_BRIDGE_HEIGHT, 28.0f);
        glm::vec3 normDir = glm::normalize(dir.direction);
        glm::vec3 lightOffset = -normDir * 18.0f;
        glm::vec3 lightSourcePos = bridgeCenter + lightOffset;
        lightSourcePos.y = 28.2f; // Structural ceiling truss level (Y=28.5)
        if (lightSourcePos.x < -16.0f) lightSourcePos.x = -16.0f;
        if (lightSourcePos.x >  16.0f) lightSourcePos.x =  16.0f;
        if (lightSourcePos.z <  -8.0f) lightSourcePos.z =  -8.0f;
        if (lightSourcePos.z >  64.0f) lightSourcePos.z =  64.0f;

        glPushMatrix();
        glTranslatef(lightSourcePos.x, lightSourcePos.y, lightSourcePos.z);

        // 1. Heavy Industrial Steel Stage Floodlight Gantry Rack
        GLCompat::setMaterial(0.18f, 0.20f, 0.24f, 40.0f);
        GLCompat::drawBox(8.0f, 1.2f, 3.2f);

        // 2. Structural safety rigging suspension arms up to ceiling
        GLCompat::setMaterial(0.12f, 0.14f, 0.16f, 30.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.95f, 0.0f);
        GLCompat::drawBox(7.2f, 0.8f, 0.4f);
        glPopMatrix();

        // 3. 4 Heavy Stage Floodlight Projectors across the gantry
        const float lampSpacing = 2.0f;
        for (int lamp = 0; lamp < 4; ++lamp) {
            float lx = -3.0f + (float)lamp * lampSpacing;
            glPushMatrix();
            glTranslatef(lx, -0.6f, 0.0f);

            // Projector body can
            GLCompat::setMaterial(0.14f, 0.16f, 0.18f, 50.0f);
            GLCompat::drawBox(1.5f, 0.8f, 1.8f);

            // Glowing Projector Lens / Reflector Dish
            glPushMatrix();
            glTranslatef(0.0f, -0.42f, 0.0f);
            if (dir.enabled) {
                GLCompat::setMaterial(1.0f, 0.98f, 0.90f, 100.0f);
                GLCompat::setEmissive(dir.color.r * 1.8f, dir.color.g * 1.8f, dir.color.b * 1.8f, 3.5f);
            } else {
                GLCompat::setMaterial(0.18f, 0.18f, 0.20f, 10.0f);
                GLCompat::resetEmission();
            }
            GLCompat::drawBox(1.35f, 0.08f, 1.6f);
            GLCompat::resetEmission();
            glPopMatrix();

            glPopMatrix();
        }

        // 4. Glowing Volumetric Stage Light Beams & Direction Indicator
        if (dir.enabled && showSunRays) {
            GLCompat::setUnlit(true);
            GLCompat::enableBlend();
            GLCompat::setDepthWrite(false);

            // (a) Broad theatrical light shaft along light vector down towards bridge
            GLCompat::setAlpha(0.15f);
            GLCompat::setMaterial(dir.color.r, dir.color.g, dir.color.b, 10.0f);
            glPushMatrix();
            glm::vec3 shaftVec = normDir * 20.0f;
            glTranslatef(shaftVec.x * 0.5f, shaftVec.y * 0.5f - 0.8f, shaftVec.z * 0.5f);
            GLCompat::drawBox(8.5f, 20.0f, 8.5f);
            glPopMatrix();

            // (b) 3D Directional Light Vector Pointer / Guide Arrow (bright cyan beacons)
            GLCompat::setAlpha(0.55f);
            GLCompat::setMaterial(0.0f, 0.88f, 1.0f, 10.0f);
            glPushMatrix();
            glTranslatef(0.0f, -1.0f, 0.0f);
            for (int seg = 1; seg <= 6; ++seg) {
                glm::vec3 arrowPos = normDir * ((float)seg * 2.8f);
                glPushMatrix();
                glTranslatef(arrowPos.x, arrowPos.y, arrowPos.z);
                GLCompat::drawSphere(0.24f, 8, 8);
                glPopMatrix();
            }
            glPopMatrix();

            GLCompat::disableBlend();
            GLCompat::setDepthWrite(true);
            GLCompat::setUnlit(false);
            GLCompat::setAlpha(1.0f);
        }

        glPopMatrix();
    }

    // Phase Switchers
    void startGlassBridge() {
        currentPhase = PHASE_GLASS_BRIDGE;
        GlassBridgeManager::init(gbSteps, gbContestants, gbShards, gbPlayerQueueNum, gbActiveQueueIndex, gbState, gbTimer);
        gbCamInitialized = false;
        sunAzimuth = 45.0f;
        sunElevation = 50.0f;
        updateSunDirection();
        AudioManager::stopAll();
    }

    void resetGlassBridge() {
        GlassBridgeManager::init(gbSteps, gbContestants, gbShards, gbPlayerQueueNum, gbActiveQueueIndex, gbState, gbTimer);
        gbCamInitialized = false;
        sunAzimuth = 45.0f;
        sunElevation = 50.0f;
        updateSunDirection();
        AudioManager::stopAll();
    }

    void setPlayerQueueFirst() {
        GlassBridgeManager::init(gbSteps, gbContestants, gbShards, gbPlayerQueueNum, gbActiveQueueIndex, gbState, gbTimer, 1);
        gbCamInitialized = false;
        sunAzimuth = 45.0f;
        sunElevation = 50.0f;
        updateSunDirection();
    }

    void startRLGL() {
        currentPhase = PHASE_RED_LIGHT_GREEN_LIGHT;
        if (gameState == WAITING) {
            gameState = PLAYING;
            AudioManager::restartSong();
        }
    }

    void resetRLGL() {
        gameTimer = GAME_TIME_LIMIT;
        aliveCount = TOTAL_CONTESTANTS;
        gameState = WAITING;

        ContestantsManager::init(player, npcs);
        DollManager::init(doll);
        GuardsManager::init(guards);
        CameraManager::init();
        particles.clear();

        keyForward = keyBackward = keyTurnLeft = keyTurnRight = keyStrafeLeft = keyStrafeRight = false;
        keyHoldSprint = keyHoldWalk = false;
        AudioManager::stopAll();
    }

    void checkPlayerElimination(bool isMoving) {
        if (gameState != PLAYING || !player.isAlive) return;

        // Win condition: crossed finish line -> unlocks Glass Bridge!
        if (player.z >= FINISH_Z) {
            gameState = WON;
            AudioManager::stopAll();
            AudioManager::playWinMusic();
            return;
        }

        // Time out
        if (gameTimer <= 0.0f) {
            gameState = TIME_UP;
            player.isAlive = false;
            aliveCount--;
            GuardsManager::triggerShot(guards, player.x, player.z);
            ContestantsManager::spawnBloodParticles(particles, player.x, player.y, player.z);
            AudioManager::playGameOverMusic();
            return;
        }

        // Red light movement check
        if (doll.lightState == RED_LIGHT && doll.headAngle > 80.0f && isMoving) {
            gameState = CAUGHT;
            player.isAlive = false;
            aliveCount--;
            GuardsManager::triggerShot(guards, player.x, player.z);
            ContestantsManager::spawnBloodParticles(particles, player.x, player.y, player.z);
            AudioManager::playEliminationSound();
            AudioManager::playGameOverMusic();
        }
    }

    void switchRLGLCamera(CameraMode newMode) {
        camMode = newMode;
        CameraManager::resetAnglesForMode(camMode);
    }

    // GLFW Callbacks
    void framebufferSizeCallback(GLFWwindow*, int w, int h) {
        windowWidth = (w > 0) ? w : 1;
        windowHeight = (h > 0) ? h : 1;
    }

    void scrollCallback(GLFWwindow*, double, double yoffset) {
        if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
            CameraManager::zoom((float)(-yoffset * 0.9), camMode);
        } else {
            gbOrbitDist -= (float)yoffset * 0.85f;
            if (gbOrbitDist < 2.5f) gbOrbitDist = 2.5f;
            if (gbOrbitDist > 55.0f) gbOrbitDist = 55.0f;
        }
    }

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return;

        if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT && gameState == WAITING) {
                    startRLGL();
                }
                isMouseDragging = true;
                glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
                if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
                    CameraManager::handleMouseClick(0, 0, (int)lastMouseX, (int)lastMouseY);
                }
            } else if (action == GLFW_RELEASE) {
                isMouseDragging = false;
                if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
                    CameraManager::handleMouseClick(0, 1, (int)lastMouseX, (int)lastMouseY);
                }
            }
        }
    }

    void cursorPosCallback(GLFWwindow*, double xpos, double ypos) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return;

        if (isMouseDragging) {
            if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
                CameraManager::handleMouseMotion((int)xpos, (int)ypos);
            } else {
                double dx = xpos - lastMouseX;
                double dy = ypos - lastMouseY;
                lastMouseX = xpos;
                lastMouseY = ypos;
                gbOrbitYaw += (float)dx * 0.40f;
                gbOrbitPitch -= (float)dy * 0.32f;
                if (gbOrbitPitch > 85.0f) gbOrbitPitch = 85.0f;
                if (gbOrbitPitch < -30.0f) gbOrbitPitch = -30.0f;
            }
        }
    }

    void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) return;

        bool isPress = (action == GLFW_PRESS || action == GLFW_REPEAT);
        keyHoldSprint = (mods & GLFW_MOD_SHIFT) != 0;
        keyHoldWalk   = (mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) != 0;

        if (action == GLFW_PRESS) {
            // Global Hotkeys
            if (key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(window, true);
                return;
            }

            // Global Lighting & Shading Hotkeys (3-Type Lighting + Gouraud/Phong switch)
            if (key == GLFW_KEY_F) {
                sceneLighting.useGouraud = !sceneLighting.useGouraud;
                Renderer::toggleShading();
                return;
            }
            if (key == GLFW_KEY_8) {
                sceneLighting.point.enabled = !sceneLighting.point.enabled;
                return;
            }
            if (key == GLFW_KEY_9) {
                sceneLighting.spot.enabled = !sceneLighting.spot.enabled;
                return;
            }
            if (key == GLFW_KEY_0) {
                sceneLighting.dir.enabled = !sceneLighting.dir.enabled;
                return;
            }

            // [P] Switch Phase between Red Light Green Light and Glass Bridge
            if (key == GLFW_KEY_P) {
                if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
                    startGlassBridge();
                } else {
                    currentPhase = PHASE_RED_LIGHT_GREEN_LIGHT;
                }
                return;
            }

            if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
                if (gameState == WAITING) {
                    if (key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER || key == GLFW_KEY_W || key == GLFW_KEY_S) {
                        startRLGL();
                    }
                }

                switch (key) {
                    case GLFW_KEY_R: resetRLGL(); break;
                    case GLFW_KEY_SPACE: if (gameState == WAITING) startRLGL(); break;
                    case GLFW_KEY_Z: player.speedTier = SPEED_WALK; break;
                    case GLFW_KEY_X: player.speedTier = SPEED_JOG; break;
                    case GLFW_KEY_C: player.speedTier = SPEED_SPRINT; break;
                    case GLFW_KEY_G:
                        if (player.speedTier == SPEED_WALK) player.speedTier = SPEED_JOG;
                        else if (player.speedTier == SPEED_JOG) player.speedTier = SPEED_SPRINT;
                        else player.speedTier = SPEED_WALK;
                        break;
                    case GLFW_KEY_1: switchRLGLCamera(CAM_THIRD_PERSON); break;
                    case GLFW_KEY_2: switchRLGLCamera(CAM_OVERHEAD); break;
                    case GLFW_KEY_3: switchRLGLCamera(CAM_TOP_DOWN); break;
                    case GLFW_KEY_4: switchRLGLCamera(CAM_DOLL_CLOSEUP); break;
                    case GLFW_KEY_5: switchRLGLCamera(CAM_SIDE_LEFT); break;
                    case GLFW_KEY_6: switchRLGLCamera(CAM_SIDE_RIGHT); break;
                    case GLFW_KEY_7: switchRLGLCamera(CAM_FINISH_LINE); break;
                    case GLFW_KEY_V: case GLFW_KEY_N: CameraManager::cycleMode(camMode); break;
                    case GLFW_KEY_F: Renderer::toggleShading(); break;
                }
            } else {
                // Glass Bridge Hotkeys
                switch (key) {
                    case GLFW_KEY_R:
                        resetGlassBridge();
                        break;
                    case GLFW_KEY_S:
                        // Skip directly to Player 456's turn
                        GlassBridgeManager::skipToPlayerTurn(gbSteps, gbContestants, gbActiveQueueIndex, gbState);
                        break;
                    case GLFW_KEY_A: case GLFW_KEY_LEFT: case GLFW_KEY_Q:
                        // JUMP LEFT GLASS PANE
                        GlassBridgeManager::jumpPlayer(gbSteps, gbContestants, gbShards, GLASS_LEFT, gbActiveQueueIndex, gbState);
                        break;
                    case GLFW_KEY_D: case GLFW_KEY_RIGHT: case GLFW_KEY_E:
                        // JUMP RIGHT GLASS PANE
                        GlassBridgeManager::jumpPlayer(gbSteps, gbContestants, gbShards, GLASS_RIGHT, gbActiveQueueIndex, gbState);
                        break;
                    case GLFW_KEY_1: gbCamMode = GB_CAM_FOLLOW; break;
                    case GLFW_KEY_2: gbCamMode = GB_CAM_ORBIT_360; break;
                    case GLFW_KEY_3: gbCamMode = GB_CAM_TOP_DOWN; break;
                    case GLFW_KEY_4: gbCamMode = GB_CAM_SIDE_PROFILE; break;
                    case GLFW_KEY_5: gbCamMode = GB_CAM_DESTINATION; break;
                    case GLFW_KEY_TAB: case GLFW_KEY_F:
                        gbFastForward = !gbFastForward;
                        break;
                }
            }
        }

        // RLGL Movement keys
        if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
            switch (key) {
                case GLFW_KEY_W: case GLFW_KEY_UP:    keyForward = isPress; break;
                case GLFW_KEY_S: case GLFW_KEY_DOWN:  keyBackward = isPress; break;
                case GLFW_KEY_A: case GLFW_KEY_LEFT:  keyTurnLeft = isPress; break;
                case GLFW_KEY_D: case GLFW_KEY_RIGHT: keyTurnRight = isPress; break;
                case GLFW_KEY_Q: keyStrafeLeft = isPress; break;
                case GLFW_KEY_E: keyStrafeRight = isPress; break;
            }
        }
    }

    void setupSquidGameImGuiStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.PopupRounding = 6.0f;
        style.WindowBorderSize = 1.8f;
        style.FrameBorderSize = 1.0f;
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.WindowPadding = ImVec2(10.0f, 10.0f);

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg]             = ImVec4(0.04f, 0.05f, 0.08f, 0.97f); // Deep obsidian black
        colors[ImGuiCol_Border]               = ImVec4(1.00f, 0.08f, 0.48f, 0.95f); // Vibrant Squid Game Hot Pink
        colors[ImGuiCol_FrameBg]              = ImVec4(0.07f, 0.10f, 0.15f, 0.95f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.12f, 0.18f, 0.25f, 1.00f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.16f, 0.24f, 0.32f, 1.00f);
        colors[ImGuiCol_TitleBg]              = ImVec4(0.05f, 0.07f, 0.11f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(1.00f, 0.08f, 0.48f, 0.90f);
        colors[ImGuiCol_Button]               = ImVec4(0.96f, 0.08f, 0.46f, 0.90f); // High-voltage pink
        colors[ImGuiCol_ButtonHovered]        = ImVec4(1.00f, 0.22f, 0.58f, 1.00f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(1.00f, 0.35f, 0.70f, 1.00f);
        colors[ImGuiCol_Header]               = ImVec4(0.02f, 0.75f, 0.55f, 0.85f); // Vibrant Tracksuit Teal
        colors[ImGuiCol_HeaderHovered]        = ImVec4(1.00f, 0.08f, 0.48f, 0.80f);
        colors[ImGuiCol_HeaderActive]         = ImVec4(1.00f, 0.08f, 0.48f, 0.95f);
        colors[ImGuiCol_Text]                 = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Crisp white
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.60f, 0.65f, 0.72f, 1.00f);
        colors[ImGuiCol_Separator]            = ImVec4(1.00f, 0.08f, 0.48f, 0.70f);
        colors[ImGuiCol_SeparatorHovered]     = ImVec4(1.00f, 0.08f, 0.48f, 0.90f);
        colors[ImGuiCol_SeparatorActive]      = ImVec4(1.00f, 0.08f, 0.48f, 1.00f);
    }
}

int main() {
    srand((unsigned int)time(NULL));

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight,
                                          "Squid Game 3D: Red Light Green Light & Glass Bridge",
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetKeyCallback(window, keyCallback);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.04f, 0.05f, 0.08f, 1.0f);

    // Modern Engine Components
    Shader shader("src/shaders/default.vert", "src/shaders/default.frag");
    Camera camera(windowWidth, windowHeight, glm::vec3(0.0f, 2.5f, START_Z - 6.0f));

    // Audio & Subsystems Initialization
    AudioManager::init();
    EnvironmentManager::init();
    resetRLGL();
    resetGlassBridge();

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    setupSquidGameImGuiStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    double lastTime = glfwGetTime();

    // -----------------------------------------------------------------------------------
    // MAIN GAME LOOP
    // -----------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double currentTime = glfwGetTime();
        float dt = (float)(currentTime - lastTime);
        lastTime = currentTime;
        if (dt > 0.05f) dt = 0.05f;

        // -------------------------------------------------------------------------------
        // UPDATE GAME LOGIC PER PHASE
        // -------------------------------------------------------------------------------
        if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
            if (gameState == PLAYING) {
                gameTimer -= dt;
                if (gameTimer <= 0.0f) {
                    gameTimer = 0.0f;
                    checkPlayerElimination(false);
                }

                DollManager::update(doll, dt);
                GuardsManager::update(guards, dt);
                ContestantsManager::update(player, npcs, guards, doll, particles, aliveCount, dt);

                if (player.isAlive) {
                    if (keyTurnLeft)  player.facingAngle -= TURN_SPEED;
                    if (keyTurnRight) player.facingAngle += TURN_SPEED;

                    float rad = player.facingAngle * 3.14159265f / 180.0f;
                    float forwardX = sinf(rad);
                    float forwardZ = cosf(rad);
                    float rightX = cosf(rad);
                    float rightZ = -sinf(rad);

                    SpeedTier effectiveTier = player.speedTier;
                    if (keyHoldSprint) effectiveTier = SPEED_SPRINT;
                    else if (keyHoldWalk) effectiveTier = SPEED_WALK;

                    float currentSpeed = SPEED_EASY_RUN;
                    float animFreq = WALK_OMEGA;
                    float animAmp = WALK_AMPLITUDE;

                    if (effectiveTier == SPEED_WALK) {
                        currentSpeed = SPEED_SLOW_WALK;
                        animFreq = WALK_OMEGA * 0.70f;
                        animAmp  = WALK_AMPLITUDE * 0.75f;
                    } else if (effectiveTier == SPEED_SPRINT) {
                        currentSpeed = SPEED_FAST_SPRINT;
                        animFreq = WALK_OMEGA * 1.45f;
                        animAmp  = WALK_AMPLITUDE * 1.30f;
                    }

                    player.speedTier = effectiveTier;
                    player.currentSpeed = currentSpeed;

                    bool isMoving = false;
                    if (keyForward) {
                        player.x += forwardX * currentSpeed;
                        player.z += forwardZ * currentSpeed;
                        isMoving = true;
                    }
                    if (keyBackward) {
                        player.x -= forwardX * currentSpeed * 0.70f;
                        player.z -= forwardZ * currentSpeed * 0.70f;
                        isMoving = true;
                    }
                    if (keyStrafeLeft) {
                        player.x -= rightX * currentSpeed * 0.80f;
                        player.z -= rightZ * currentSpeed * 0.80f;
                        isMoving = true;
                    }
                    if (keyStrafeRight) {
                        player.x += rightX * currentSpeed * 0.80f;
                        player.z += rightZ * currentSpeed * 0.80f;
                        isMoving = true;
                    }

                    if (player.x < -ARENA_HALF_WIDTH + 0.8f) player.x = -ARENA_HALF_WIDTH + 0.8f;
                    if (player.x >  ARENA_HALF_WIDTH - 0.8f) player.x =  ARENA_HALF_WIDTH - 0.8f;
                    if (player.z < -ARENA_HALF_LENGTH + 0.8f) player.z = -ARENA_HALF_LENGTH + 0.8f;

                    if (isMoving) {
                        player.walkTime += dt;
                        player.legAngle = animAmp * sinf(animFreq * player.walkTime);
                        player.armAngle = -player.legAngle;
                    } else {
                        player.legAngle = 0.0f;
                        player.armAngle = 0.0f;
                    }

                    checkPlayerElimination(isMoving);
                } else {
                    if (player.deathTimer < 1.0f) {
                        player.deathTimer += dt * 3.0f;
                        if (player.deathTimer > 1.0f) player.deathTimer = 1.0f;
                    }
                }
            }

            ContestantsManager::updateParticles(particles, dt);
            CameraManager::update(player, doll, camMode, dt);
        } else {
            // Glass Bridge Phase update
            if (gbState == GB_PLAYING) {
                gbTimer -= dt;
                if (gbTimer <= 0.0f) {
                    gbTimer = 0.0f;
                    gbState = GB_ELIMINATED;
                }
            }
            GlassBridgeManager::update(gbSteps, gbContestants, gbShards, gbActiveQueueIndex, gbState, dt, gbFastForward);
        }

        // =========================================================================
        // PASS 1: MAIN 3D SCENE VIEWPORT (Center of screen between sidebars)
        // =========================================================================
        int leftBarW = UI_LEFT_BAR_WIDTH;
        int rightBarW = UI_RIGHT_BAR_WIDTH;
        int centerW = windowWidth - leftBarW - rightBarW;
        if (centerW < 300) {
            leftBarW = 0; rightBarW = 0; centerW = windowWidth;
        }

        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(leftBarW, 0, centerW, windowHeight);

        float eyeX, eyeY, eyeZ, lookX, lookY, lookZ, upX, upY, upZ, fov;

        if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
            CameraManager::getViewVectors(player, doll, camMode,
                                          eyeX, eyeY, eyeZ, lookX, lookY, lookZ, upX, upY, upZ, fov);
            camera.width = centerW;
            camera.height = windowHeight;
            camera.SetView(glm::vec3(eyeX, eyeY, eyeZ),
                           glm::vec3(lookX, lookY, lookZ),
                           glm::vec3(upX, upY, upZ),
                           fov, 0.1f, 280.0f);

            // -------------------------------------------------------------
            // DYNAMIC 3-TYPE LIGHTING (RLGL Arena)
            // -------------------------------------------------------------
            // 1. Directional Light: Sun Key Light
            updateSunDirection();
            sceneLighting.dir.color = glm::vec3(0.82f, 0.78f, 0.72f);

            // 2. Point Light: High Arena Floodlight Tower
            sceneLighting.point.position = glm::vec3(18.0f, 16.0f, 15.0f);
            sceneLighting.point.color = glm::vec3(1.0f, 0.88f, 0.62f);
            sceneLighting.point.intensity = 1.35f;

            // 3. Spot Light: Young-hee Doll Eye Scanning Searchlight
            float rad = glm::radians(doll.headAngle);
            sceneLighting.spot.position = glm::vec3(doll.x, doll.y + 2.5f * DOLL_SCALE, doll.z);
            sceneLighting.spot.direction = glm::normalize(glm::vec3(sinf(rad), -0.12f, cosf(rad)));
            if (doll.lightState == RED_LIGHT) {
                sceneLighting.spot.color = glm::vec3(1.0f, 0.15f, 0.15f); // Red scanning beam
                sceneLighting.spot.intensity = 2.8f;
            } else {
                sceneLighting.spot.color = glm::vec3(0.35f, 0.90f, 0.40f); // Soft green beam while singing
                sceneLighting.spot.intensity = 1.2f;
            }

            GLCompat::beginFrame(shader, camera, sceneLighting);

            EnvironmentManager::draw(gameTimer);
            DollManager::draw(doll);
            GuardsManager::draw(guards);
            ContestantsManager::drawNPCs(npcs);
            ContestantsManager::drawPlayer(player);
            ContestantsManager::drawParticles(particles);

            // Draw physical light source fixtures: Sun celestial orb & Arena Floodlight Tower
            drawSun(sceneLighting.dir);
            drawArenaFloodlight(sceneLighting.point);
        } else {
            // Glass Bridge Scene with Exponential Camera Smoothing
            float rawEyeX, rawEyeY, rawEyeZ;
            float rawLookX, rawLookY, rawLookZ;
            float rawUpX, rawUpY, rawUpZ, rawFov;

            GlassBridgeManager::getCameraVectors(gbContestants, gbActiveQueueIndex, gbCamMode,
                                                gbOrbitYaw, gbOrbitPitch, gbOrbitDist,
                                                rawEyeX, rawEyeY, rawEyeZ,
                                                rawLookX, rawLookY, rawLookZ,
                                                rawUpX, rawUpY, rawUpZ, rawFov);

            if (!gbCamInitialized) {
                gbSmoothEye = glm::vec3(rawEyeX, rawEyeY, rawEyeZ);
                gbSmoothLook = glm::vec3(rawLookX, rawLookY, rawLookZ);
                gbCamInitialized = true;
            } else {
                float camSpeed = (gbCamMode == GB_CAM_FOLLOW || gbCamMode == GB_CAM_OVER_SHOULDER) ? 9.5f : 7.5f;
                float factor = 1.0f - expf(-camSpeed * dt);
                if (factor > 1.0f) factor = 1.0f;
                gbSmoothEye = glm::mix(gbSmoothEye, glm::vec3(rawEyeX, rawEyeY, rawEyeZ), factor);
                gbSmoothLook = glm::mix(gbSmoothLook, glm::vec3(rawLookX, rawLookY, rawLookZ), factor);
            }

            camera.width = centerW;
            camera.height = windowHeight;
            camera.SetView(gbSmoothEye,
                           gbSmoothLook,
                           glm::vec3(rawUpX, rawUpY, rawUpZ),
                           rawFov, 0.1f, 300.0f);

            // -------------------------------------------------------------
            // DYNAMIC 3-TYPE LIGHTING (Glass Bridge)
            // -------------------------------------------------------------
            // 1. Directional Light: Stage Overhead Key Light
            updateSunDirection();
            sceneLighting.dir.color = glm::vec3(0.85f, 0.82f, 0.76f);

            // 2. Point Light: Suspended Glass Bridge Chandelier at bridge center
            sceneLighting.point.position = glm::vec3(0.0f, GB_BRIDGE_HEIGHT + 7.0f, 28.0f);
            sceneLighting.point.color = glm::vec3(1.0f, 0.88f, 0.65f);
            sceneLighting.point.intensity = 1.6f;

            // 3. Spot Light: Theatrical Follow-Spotlight tracking active jumper
            glm::vec3 activePos(0.0f, GB_BRIDGE_HEIGHT, 0.0f);
            if (gbActiveQueueIndex < (int)gbContestants.size()) {
                activePos = glm::vec3(gbContestants[gbActiveQueueIndex].x,
                                      gbContestants[gbActiveQueueIndex].y,
                                      gbContestants[gbActiveQueueIndex].z);
            }
            sceneLighting.spot.position = glm::vec3(activePos.x, GB_BRIDGE_HEIGHT + 8.5f, activePos.z);
            sceneLighting.spot.direction = glm::vec3(0.0f, -1.0f, 0.0f);
            sceneLighting.spot.color = glm::vec3(0.85f, 0.95f, 1.0f); // Crisp theatrical follow beam
            sceneLighting.spot.intensity = 3.2f;

            GLCompat::beginFrame(shader, camera, sceneLighting);

            GlassBridgeManager::drawEnvironment((float)currentTime);
            GlassBridgeManager::drawBridge(gbSteps, (float)currentTime, gbContestants, gbActiveQueueIndex);
            GlassBridgeManager::drawContestants(gbContestants, gbActiveQueueIndex);
            GlassBridgeManager::drawShards(gbShards);

            // Draw physical light fixtures: Overhead Stage Floodlights, Chandelier & Follow-Spotlight
            drawStageSunBank(sceneLighting.dir);
            drawBridgeChandelier(sceneLighting.point);
            drawFollowSpotlight(sceneLighting.spot);
        }

        // =========================================================================
        // PASS 2: CORNER CCTV SURVEILLANCE PIP (CAM 01: DOLL) - RLGL Only
        // =========================================================================
        if (leftBarW > 100 && currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
            int pipPad = 12;
            int pipW = leftBarW - pipPad * 2;
            int pipH = (int)(pipW * 0.85f);
            int pipX = pipPad;
            int pipY = windowHeight - pipH - 105;

            glEnable(GL_SCISSOR_TEST);
            glScissor(pipX, pipY, pipW, pipH);
            glViewport(pipX, pipY, pipW, pipH);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glClearColor(0.26f, 0.52f, 0.82f, 1.0f);
            CameraManager::getDollCloseupVectors(doll, eyeX, eyeY, eyeZ, lookX, lookY, lookZ, upX, upY, upZ, fov);
            camera.width = pipW;
            camera.height = pipH;
            camera.SetView(glm::vec3(eyeX, eyeY, eyeZ),
                           glm::vec3(lookX, lookY, lookZ),
                           glm::vec3(upX, upY, upZ),
                           fov, 0.05f, 260.0f);

            SceneLighting pipLighting = sceneLighting;
            pipLighting.dir.direction = glm::vec3(-0.35f, -1.0f, -0.4f);
            pipLighting.dir.color = glm::vec3(1.15f, 1.12f, 1.08f);

            GLCompat::beginFrame(shader, camera, pipLighting);
            EnvironmentManager::draw(false);
            DollManager::draw(doll);
            GuardsManager::draw(guards);
            ContestantsManager::drawNPCs(npcs);
            ContestantsManager::drawPlayer(player);

            glDisable(GL_SCISSOR_TEST);
            glClearColor(0.04f, 0.05f, 0.08f, 1.0f);
        }

        // =========================================================================
        // PASS 3: MODERN DEAR IMGUI HUD & INTERACTIVE CONTROLS
        // =========================================================================
        glViewport(0, 0, windowWidth, windowHeight);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 0. TOP PHASE NAVIGATOR & SQUID GAME BROADCAST HEADER (Glass Bridge Phase Only)
        if (currentPhase == PHASE_GLASS_BRIDGE) {
            ImGui::SetNextWindowPos(ImVec2((float)leftBarW + 12.0f, 6.0f));
            ImGui::SetNextWindowSize(ImVec2((float)centerW - 24.0f, 68.0f));
            ImGui::Begin("TopNavBar", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);

            // Row 1: Squid Game Brand Header with Glowing Gold Piggy Bank Bounty
            ImGui::TextColored(ImVec4(1.00f, 0.08f, 0.48f, 1.0f), "S Q U I D   G A M E");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "|  ROUND 5: GLASS BRIDGE");

            // Piggy Bank Prize Pool Counter: ₩ 45,600,000,000 (₩45.6 Billion)
            int aliveGB = 0;
            for (const auto& c : gbContestants) if (c.isAlive) aliveGB++;
            int elimGB = GB_TOTAL_PLAYERS - aliveGB;
            char prizeBuf[80];
            if (elimGB > 0) {
                sprintf(prizeBuf, "PIGGY BANK: ₩ 45,600,000,000 | BOUNTY: +₩%d00M", elimGB);
            } else {
                sprintf(prizeBuf, "PIGGY BANK: ₩ 45,600,000,000");
            }
            float prizeWidth = ImGui::CalcTextSize(prizeBuf).x;
            ImGui::SameLine(ImGui::GetWindowWidth() - prizeWidth - 10.0f);
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "%s", prizeBuf);

            ImGui::Spacing();

            // Row 2: Phase Switch Tabs
            float tabW = ((float)centerW - 35.0f) * 0.5f;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.08f, 0.12f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.08f, 0.48f, 0.9f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
            if (ImGui::Button("<< ROUND 1: RED LIGHT, GREEN LIGHT", ImVec2(tabW, 30))) {
                currentPhase = PHASE_RED_LIGHT_GREEN_LIGHT;
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00f, 0.08f, 0.48f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::Button("ROUND 5: GLASS BRIDGE", ImVec2(tabW, 30));
            ImGui::PopStyleColor(2);

            ImGui::End();
        }

        // 1. LEFT SIDEBAR PANEL (Doll Cam feed frame + Live Stats)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float)leftBarW, (float)windowHeight));
            ImGui::Begin("LeftPanel", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);

            int pipPad = 12;
            int pipW = leftBarW - pipPad * 2;
            int pipH = (int)(pipW * 0.85f);

            if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
                // Title & Subtitle Header
                ImGui::TextColored(ImVec4(1.00f, 0.08f, 0.48f, 1.0f), "SQUID GAME");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "| RED LIGHT, GREEN LIGHT");
                ImGui::Spacing();

                // Doll Cam Header with State Indicator
                if (doll.lightState == RED_LIGHT) {
                    ImGui::TextColored(ImVec4(1.0f, 0.15f, 0.15f, 1.0f), "● SCANNING");
                } else {
                    ImGui::TextColored(ImVec4(0.10f, 0.95f, 0.45f, 1.0f), "● CHANTING");
                }
                ImGui::SameLine(ImGui::GetWindowWidth() - 95.0f);
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.95f, 1.0f), "CAM 01 : DOLL");

                // Empty space reserved for 3D PiP viewport with CCTV corner brackets
                ImVec2 pipMin = ImGui::GetCursorScreenPos();
                ImVec2 pipMax(pipMin.x + (float)pipW, pipMin.y + (float)pipH);
                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRect(pipMin, pipMax, IM_COL32(0, 220, 240, 180), 2.0f, 0, 1.5f);
                float cLen = 14.0f;
                dl->AddLine(pipMin, ImVec2(pipMin.x + cLen, pipMin.y), IM_COL32(0, 255, 255, 255), 2.5f);
                dl->AddLine(pipMin, ImVec2(pipMin.x, pipMin.y + cLen), IM_COL32(0, 255, 255, 255), 2.5f);
                dl->AddLine(ImVec2(pipMax.x, pipMin.y), ImVec2(pipMax.x - cLen, pipMin.y), IM_COL32(0, 255, 255, 255), 2.5f);
                dl->AddLine(ImVec2(pipMax.x, pipMin.y), ImVec2(pipMax.x, pipMin.y + cLen), IM_COL32(0, 255, 255, 255), 2.5f);
                dl->AddLine(ImVec2(pipMin.x, pipMax.y), ImVec2(pipMin.x + cLen, pipMax.y), IM_COL32(0, 255, 255, 255), 2.5f);
                dl->AddLine(ImVec2(pipMin.x, pipMax.y), ImVec2(pipMin.x, pipMax.y - cLen), IM_COL32(0, 255, 255, 255), 2.5f);
                dl->AddLine(pipMax, ImVec2(pipMax.x - cLen, pipMax.y), IM_COL32(0, 255, 255, 255), 2.5f);
                dl->AddLine(pipMax, ImVec2(pipMax.x, pipMax.y - cLen), IM_COL32(0, 255, 255, 255), 2.5f);

                ImGui::Dummy(ImVec2((float)pipW, (float)pipH + 6.0f));

                // Cyan Bordered Digital Clock Box
                int minutes = (int)gameTimer / 60;
                int seconds = (int)gameTimer % 60;
                char clockBuf[32];
                sprintf(clockBuf, "    0 %d : %02d", minutes, seconds);

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.85f, 1.0f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("ClockBox", ImVec2(-1, 46), true)) {
                    ImGui::SetCursorPosY(12.0f);
                    ImGui::TextColored(ImVec4(1.0f, 0.15f, 0.20f, 1.0f), "%s", clockBuf);
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                ImGui::Spacing();

                // Pink Bordered Players Count Box
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.08f, 0.48f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("PlayerBox", ImVec2(-1, 40), true)) {
                    ImGui::SetCursorPosY(8.0f);
                    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "PLAYERS: %d / %d", aliveCount, TOTAL_CONTESTANTS);
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                ImGui::Spacing();

                // Green / Red Light Status Banner
                if (gameState == WAITING || doll.lightState == GREEN_LIGHT) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.02f, 0.80f, 0.45f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.05f, 0.90f, 0.55f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.02f, 0.80f, 0.45f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::Button("GREEN LIGHT - MOVE!", ImVec2(-1, 36));
                    ImGui::PopStyleColor(4);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.08f, 0.15f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.00f, 0.15f, 0.22f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.95f, 0.08f, 0.15f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::Button("RED LIGHT - FREEZE!", ImVec2(-1, 36));
                    ImGui::PopStyleColor(4);
                }
                ImGui::Spacing();

                // Course Progress
                float totalDist = FINISH_Z - START_Z;
                float currDist = player.z - START_Z;
                float progress = currDist / totalDist;
                if (progress < 0.0f) progress = 0.0f;
                if (progress > 1.0f) progress = 1.0f;

                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "COURSE PROGRESS");
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.00f, 0.08f, 0.48f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.12f, 0.16f, 1.0f));
                ImGui::ProgressBar(progress, ImVec2(-1, 14), "");
                ImGui::PopStyleColor(2);
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "%d%% TO FINISH LINE", (int)(progress * 100.0f));
                ImGui::Spacing();

                // Survival Radar Mini-Map
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "ARENA RADAR");
                ImVec2 radarPos = ImGui::GetCursorScreenPos();
                float radarW = (float)pipW;
                float radarH = 110.0f;
                dl->AddRectFilled(radarPos, ImVec2(radarPos.x + radarW, radarPos.y + radarH), IM_COL32(10, 12, 16, 250), 4.0f);
                dl->AddRect(radarPos, ImVec2(radarPos.x + radarW, radarPos.y + radarH), IM_COL32(0, 210, 255, 200), 4.0f, 0, 1.8f);

                // Finish Line marker on radar
                float finishScreenY = radarPos.y + 12.0f;
                dl->AddLine(ImVec2(radarPos.x + 8.0f, finishScreenY), ImVec2(radarPos.x + radarW - 8.0f, finishScreenY), IM_COL32(255, 20, 120, 240), 2.5f);

                // Draw NPCs on radar
                for (const auto& npc : npcs) {
                    if (!npc.isAlive) continue;
                    float normX = (npc.x + ARENA_HALF_WIDTH) / (ARENA_HALF_WIDTH * 2.0f);
                    float normZ = (npc.z - START_Z) / totalDist;
                    if (normX < 0.0f) normX = 0.0f; if (normX > 1.0f) normX = 1.0f;
                    if (normZ < 0.0f) normZ = 0.0f; if (normZ > 1.0f) normZ = 1.0f;
                    float dotX = radarPos.x + 10.0f + normX * (radarW - 20.0f);
                    float dotY = radarPos.y + radarH - 12.0f - normZ * (radarH - 24.0f);
                    dl->AddCircleFilled(ImVec2(dotX, dotY), 2.5f, IM_COL32(0, 235, 120, 220));
                }

                // Draw Player 456 on radar
                {
                    float normX = (player.x + ARENA_HALF_WIDTH) / (ARENA_HALF_WIDTH * 2.0f);
                    float normZ = (player.z - START_Z) / totalDist;
                    if (normX < 0.0f) normX = 0.0f; if (normX > 1.0f) normX = 1.0f;
                    if (normZ < 0.0f) normZ = 0.0f; if (normZ > 1.0f) normZ = 1.0f;
                    float dotX = radarPos.x + 10.0f + normX * (radarW - 20.0f);
                    float dotY = radarPos.y + radarH - 12.0f - normZ * (radarH - 24.0f);
                    dl->AddCircleFilled(ImVec2(dotX, dotY), 5.0f, IM_COL32(255, 220, 0, 255));
                    dl->AddCircle(ImVec2(dotX, dotY), 7.0f, IM_COL32(255, 30, 90, 255), 0, 1.8f);
                }

                ImGui::Dummy(ImVec2(radarW, radarH + 4.0f));
                ImGui::Spacing();

                // Pace Indicator
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.85f, 1.0f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.8f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (player.speedTier == SPEED_WALK) {
                    if (ImGui::BeginChild("PaceBox", ImVec2(-1, 32), true)) {
                        ImGui::SetCursorPosY(6.0f);
                        ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "PACE: WALKING (0.5x)");
                    }
                    ImGui::EndChild();
                } else if (player.speedTier == SPEED_JOG) {
                    if (ImGui::BeginChild("PaceBox", ImVec2(-1, 32), true)) {
                        ImGui::SetCursorPosY(6.0f);
                        ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.50f, 1.0f), "PACE: JOGGING (1.0x)");
                    }
                    ImGui::EndChild();
                } else {
                    if (ImGui::BeginChild("PaceBox", ImVec2(-1, 32), true)) {
                        ImGui::SetCursorPosY(6.0f);
                        ImGui::TextColored(ImVec4(1.0f, 0.20f, 0.35f, 1.0f), "PACE: SPRINTING (1.7x)");
                    }
                    ImGui::EndChild();
                }
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
            } else {
                // Glass Bridge Left Panel: Clean, spacious, Squid Game aesthetic (no clashing CCTV)
                ImGui::TextColored(ImVec4(1.00f, 0.08f, 0.48f, 1.0f), "SQUID GAME");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 0.90f, 1.0f, 1.0f), "| GLASS BRIDGE");
                ImGui::Separator();
                ImGui::Spacing();

                // Cyan Bordered Digital Clock Box
                int minutes = (int)gbTimer / 60;
                int seconds = (int)gbTimer % 60;
                char clockBuf[32];
                sprintf(clockBuf, "   %02d : %02d", minutes, seconds);

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.85f, 1.0f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("ClockBox", ImVec2(-1, 54), true)) {
                    ImGui::SetCursorPosY(6.0f);
                    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "REMAINING TIME");
                    ImGui::TextColored(ImVec4(1.0f, 0.20f, 0.25f, 1.0f), "%s", clockBuf);
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                ImGui::Spacing();

                // Pink Bordered Players Count Box
                int aliveGB = 0;
                for (const auto& c : gbContestants) if (c.isAlive) aliveGB++;
                int elimGB = GB_TOTAL_PLAYERS - aliveGB;

                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.08f, 0.48f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("PlayerBox", ImVec2(-1, 58), true)) {
                    ImGui::SetCursorPosY(6.0f);
                    ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.65f, 1.0f), "PLAYER 456 (QUEUE #%02d)", gbPlayerQueueNum);
                    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "ALIVE: %d / 16  |  FALLEN: %d", aliveGB, elimGB);
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                ImGui::Spacing();

                // Glass Bridge Turn Status Banner & Controls
                bool isP456Turn = (gbActiveQueueIndex < (int)gbContestants.size() && gbContestants[gbActiveQueueIndex].isPlayer456);

                if (isP456Turn) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.80f, 0.50f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::Button("YOUR TURN: CHOOSE A PANEL", ImVec2(-1, 38));
                    ImGui::PopStyleColor(2);

                    ImGui::Spacing();
                    float halfW = (ImGui::GetWindowWidth() - 28.0f) * 0.5f;

                    // Luminous Cyan Left Button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.65f, 0.95f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.85f, 1.0f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.95f, 1.0f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
                    if (ImGui::Button("LEFT (A)", ImVec2(halfW, 46))) {
                        GlassBridgeManager::jumpPlayer(gbSteps, gbContestants, gbShards, GLASS_LEFT, gbActiveQueueIndex, gbState);
                    }
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(3);

                    ImGui::SameLine();

                    // Vibrant Magenta Right Button
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.96f, 0.08f, 0.48f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.25f, 0.65f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.50f, 0.80f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
                    if (ImGui::Button("RIGHT (D)", ImVec2(halfW, 46))) {
                        GlassBridgeManager::jumpPlayer(gbSteps, gbContestants, gbShards, GLASS_RIGHT, gbActiveQueueIndex, gbState);
                    }
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(3);
                } else {
                    int currentJumper = (gbActiveQueueIndex < (int)gbContestants.size()) ? gbContestants[gbActiveQueueIndex].queueNumber : 16;
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.90f, 0.55f, 0.05f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    char queueBuf[64];
                    sprintf(queueBuf, "JUMPER: #%02d | YOU: #%02d", currentJumper, gbPlayerQueueNum);
                    ImGui::Button(queueBuf, ImVec2(-1, 34));
                    ImGui::PopStyleColor(2);

                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.96f, 0.08f, 0.48f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.22f, 0.58f, 1.0f));
                    if (ImGui::Button("SKIP TO MY TURN (S)", ImVec2(-1, 36))) {
                        GlassBridgeManager::skipToPlayerTurn(gbSteps, gbContestants, gbActiveQueueIndex, gbState);
                    }
                    ImGui::PopStyleColor(2);

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.02f, 0.55f, 0.45f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.05f, 0.70f, 0.58f, 1.0f));
                    if (ImGui::Button("PLAY FIRST AS #01", ImVec2(-1, 30))) {
                        setPlayerQueueFirst();
                    }
                    ImGui::PopStyleColor(2);
                }
                ImGui::Spacing();

                // Bridge Step Progress
                int currStep = (gbActiveQueueIndex < (int)gbContestants.size()) ? gbContestants[gbActiveQueueIndex].currentStep : 16;
                float gbProgress = (float)(currStep + 1) / (float)GB_NUM_STEPS;
                if (gbProgress < 0.0f) gbProgress = 0.0f;
                if (gbProgress > 1.0f) gbProgress = 1.0f;

                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "BRIDGE PROGRESS");
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.85f, 1.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.12f, 0.16f, 1.0f));
                ImGui::ProgressBar(gbProgress, ImVec2(-1, 16), "");
                ImGui::PopStyleColor(2);
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "STEP %d / %d (%d%%)", currStep + 1, GB_NUM_STEPS, (int)(gbProgress * 100.0f));
                ImGui::Spacing();

                // Fast Forward AI Toggle Button
                if (gbFastForward) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.02f, 0.85f, 0.45f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    if (ImGui::Button(">> FAST-FORWARD: ON (TAB/F)", ImVec2(-1, 34))) {
                        gbFastForward = false;
                    }
                    ImGui::PopStyleColor(2);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.80f, 1.0f, 0.8f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
                    if (ImGui::Button(">> FAST-FORWARD AI (TAB/F)", ImVec2(-1, 34))) {
                        gbFastForward = true;
                    }
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(2);
                }
            }

            ImGui::End();
        }

        // 2. RIGHT SIDEBAR PANEL
        {
            ImGui::SetNextWindowPos(ImVec2((float)(windowWidth - rightBarW), 0));
            ImGui::SetNextWindowSize(ImVec2((float)rightBarW, (float)windowHeight));
            ImGui::Begin("RightPanel", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

            if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
                ImGui::TextColored(ImVec4(1.00f, 0.08f, 0.48f, 1.0f), "CONTROLS");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "| RLGL");
                ImGui::Separator();
                ImGui::Spacing();

                // CARD 1: CAMERA ANGLES (7 Modes, highlighted active)
                ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "CAMERA ANGLES (KEY)");
                ImGui::Spacing();

                struct CamItem {
                    CameraMode mode;
                    const char* label;
                };

                const CamItem camItems[] = {
                    { CAM_THIRD_PERSON, ">> [1] 3RD PERSON VIEW" },
                    { CAM_OVERHEAD,     ">> [2] ARENA FREE 360° CAM" },
                    { CAM_TOP_DOWN,     ">> [3] TOP ARENA VIEW" },
                    { CAM_DOLL_CLOSEUP, ">> [4] DOLL 360° ORBIT" },
                    { CAM_SIDE_LEFT,    ">> [5] LEFT BROADCAST" },
                    { CAM_SIDE_RIGHT,   ">> [6] RIGHT BROADCAST" },
                    { CAM_FINISH_LINE,  ">> [7] FINISH LINE 360°" }
                };

                const int numCamItems = sizeof(camItems) / sizeof(camItems[0]);
                for (int i = 0; i < numCamItems; ++i) {
                    bool isSelected = (camMode == camItems[i].mode);
                    if (isSelected) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.02f, 0.85f, 0.45f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.05f, 0.95f, 0.55f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.02f, 0.85f, 0.45f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        if (ImGui::Button(camItems[i].label, ImVec2(-1, 26))) {
                            switchRLGLCamera(camItems[i].mode);
                        }
                        ImGui::PopStyleColor(4);
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.06f, 0.09f, 0.95f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.15f, 0.20f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.00f, 0.08f, 0.48f, 0.4f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.08f, 0.48f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.84f, 0.0f, 1.0f));
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
                        const char* inactiveLabel = camItems[i].label + 3; // Strip ">> "
                        if (ImGui::Button(inactiveLabel, ImVec2(-1, 26))) {
                            switchRLGLCamera(camItems[i].mode);
                        }
                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor(5);
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // CARD 2: SPEED & MOVEMENT
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.85f, 1.0f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.8f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("MoveCard", ImVec2(-1, 52), true)) {
                    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "W / S : FORWARD / BACK");
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "A / D : TURN LEFT / RIGHT");
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                ImGui::Spacing();

                // CARD 3: SPEED TIERS
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.02f, 0.85f, 0.45f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.8f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("SpeedCard", ImVec2(-1, 52), true)) {
                    ImGui::TextColored(ImVec4(0.0f, 0.95f, 0.50f, 1.0f), "Z/X/C : WALK / JOG / SPRINT");
                    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "G     : CYCLE SPEED GEAR");
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                ImGui::Spacing();

                // CARD 4: SYSTEM CONTROLS
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.08f, 0.48f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.8f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("SystemCard", ImVec2(-1, 68), true)) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "MOUSE DRAG  : 360° ORBIT");
                    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "MOUSE WHEEL : ZOOM IN / OUT");
                    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "P : GLASS BRIDGE | R : RESTART");
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
            } else {
                // Glass Bridge Right Panel (Squid Game styling & cameras)
                ImGui::TextColored(ImVec4(1.00f, 0.08f, 0.48f, 1.0f), "COMMAND CENTER");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "| BRIDGE");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "GLASS BRIDGE CAMERAS");
                ImGui::Spacing();

                struct GBCamItem { GlassBridgeCamMode mode; const char* label; };
                const GBCamItem gbCamItems[] = {
                    { GB_CAM_FOLLOW,        "[1] 3RD PERSON FOLLOW" },
                    { GB_CAM_ORBIT_360,     "[2] FREE 360° ORBIT" },
                    { GB_CAM_TOP_DOWN,      "[3] TOP-DOWN AERIAL" },
                    { GB_CAM_SIDE_PROFILE,  "[4] SIDE PROFILE VIEW" },
                    { GB_CAM_DESTINATION,   "[5] DESTINATION CAM" }
                };

                for (int i = 0; i < 5; ++i) {
                    bool isSelected = (gbCamMode == gbCamItems[i].mode);
                    if (isSelected) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.75f, 0.95f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        if (ImGui::Button(gbCamItems[i].label, ImVec2(-1, 28))) gbCamMode = gbCamItems[i].mode;
                        ImGui::PopStyleColor(2);
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.05f, 0.06f, 0.09f, 0.95f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.85f, 1.0f, 0.9f));
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.84f, 0.0f, 1.0f));
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
                        if (ImGui::Button(gbCamItems[i].label, ImVec2(-1, 28))) gbCamMode = gbCamItems[i].mode;
                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor(3);
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Controls Card
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.85f, 1.0f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.8f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("ActionCard", ImVec2(-1, 84), true)) {
                    ImGui::TextColored(ImVec4(0.0f, 0.88f, 1.0f, 1.0f), "A / LEFT  : JUMP LEFT");
                    ImGui::TextColored(ImVec4(1.00f, 0.08f, 0.48f, 1.0f), "D / RIGHT : JUMP RIGHT");
                    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "S : SKIP TO MY TURN");
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
                ImGui::Spacing();

                // System Card
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.00f, 0.08f, 0.48f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.8f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("SystemCard", ImVec2(-1, 68), true)) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "MOUSE DRAG  : 360° ORBIT");
                    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "MOUSE WHEEL : ZOOM IN / OUT");
                    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "P : SWITCH GAME | R : RESTART");
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(2);
            }

            // =========================================================================
            // CARD: 3-TYPE LIGHTING & SHADING LAB
            // =========================================================================
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.05f, 0.08f, 0.98f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.84f, 0.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);

            if (ImGui::BeginChild("LightingCard", ImVec2(-1, 335), true)) {
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "LIGHTING & SHADING LAB");
                ImGui::Spacing();

                // 1. Shading Model Switch (Gouraud vs Phong)
                bool isGouraud = sceneLighting.useGouraud;
                if (isGouraud) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.45f, 0.05f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    if (ImGui::Button("[F] GOURAUD (PER-VERTEX)", ImVec2(-1, 26))) {
                        sceneLighting.useGouraud = false;
                    }
                    ImGui::PopStyleColor(2);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.02f, 0.80f, 0.45f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    if (ImGui::Button("[F] PHONG (PER-PIXEL)", ImVec2(-1, 26))) {
                        sceneLighting.useGouraud = true;
                    }
                    ImGui::PopStyleColor(2);
                }

                ImGui::Spacing();

                // 2. 3 Types of Lighting Toggles & Controls
                // (a) Directional Light (Sun / Key Light)
                const char* dirName = (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) ?
                                      "1. Directional Sun [0]" : "1. Directional Stage [0]";
                if (ImGui::Checkbox(dirName, &sceneLighting.dir.enabled)) {}
                if (sceneLighting.dir.enabled) {
                    ImGui::Indent(10.0f);
                    if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
                        ImGui::TextColored(ImVec4(1.0f, 0.88f, 0.40f, 1.0f), "Sun Sky Position:");
                        if (ImGui::SliderFloat("Azimuth##Sun", &sunAzimuth, 0.0f, 360.0f, "%.0f deg")) {
                            updateSunDirection();
                        }
                        if (ImGui::SliderFloat("Elevation##Sun", &sunElevation, 10.0f, 85.0f, "%.0f deg")) {
                            updateSunDirection();
                        }
                        ImGui::Checkbox("Sun Rays & Cone", &showSunRays);
                        ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.65f, 1.0f), "Dir: [%.2f, %.2f, %.2f]",
                                           sceneLighting.dir.direction.x, sceneLighting.dir.direction.y, sceneLighting.dir.direction.z);
                        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.90f, 1.0f), "-> Look up & right to see 3D Sun!");
                    } else {
                        ImGui::TextColored(ImVec4(1.0f, 0.88f, 0.40f, 1.0f), "Overhead Floodlight Array:");
                        if (ImGui::SliderFloat("Azimuth##GBSun", &sunAzimuth, 0.0f, 360.0f, "%.0f deg")) {
                            updateSunDirection();
                        }
                        if (ImGui::SliderFloat("Elevation##GBSun", &sunElevation, 15.0f, 85.0f, "%.0f deg")) {
                            updateSunDirection();
                        }
                        ImGui::Checkbox("Beams & Vector Guide", &showSunRays);
                        ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.65f, 1.0f), "Dir: [%.2f, %.2f, %.2f]",
                                           sceneLighting.dir.direction.x, sceneLighting.dir.direction.y, sceneLighting.dir.direction.z);
                        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.90f, 1.0f), "Overhead Truss Rig: Look UP to see!");
                    }
                    ImGui::Unindent(10.0f);
                }

                ImGui::Spacing();

                // (b) Point Light (Omnidirectional + Attenuation)
                const char* ptName = (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) ?
                                     "2. Arena Tower Pt [8]" : "2. Chandelier Pt [8]";
                if (ImGui::Checkbox(ptName, &sceneLighting.point.enabled)) {}

                // (c) Spot Light (Cone Cutoff + Attenuation)
                const char* spName = (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) ?
                                     "3. Doll Eye Spot [9]" : "3. Jumper Spot [9]";
                if (ImGui::Checkbox(spName, &sceneLighting.spot.enabled)) {}

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.70f, 0.70f, 0.75f, 1.0f), "Hotkeys: F:Shade | 0/8/9:Lights");
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            ImGui::End();
        }

        // 3. CENTER SCREEN STATE OVERLAYS (Waiting / Eliminated / Won)
        if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
            if (gameState == WAITING) {
                ImVec2 center(windowWidth * 0.5f, windowHeight * 0.5f);
                ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::Begin("StartOverlay", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

                ImGui::TextColored(ImVec4(1.00f, 0.08f, 0.48f, 1.0f), "=== SQUID GAME: ROUND 1 ===");
                ImGui::Spacing();
                ImGui::Text("Advance to the Pink Finish Line before the 60s timer expires.");
                ImGui::Text("Only move when the Doll is facing the tree (Green Light).");
                ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "FREEZE IMMEDIATELY when she turns her head (Red Light)!");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.02f, 0.85f, 0.45f, 1.0f));
                if (ImGui::Button("START GAME (SPACE / CLICK)", ImVec2(-1, 40))) {
                    startRLGL();
                }
                ImGui::PopStyleColor();

                ImGui::End();
            } else if (gameState == CAUGHT || gameState == TIME_UP) {
                ImVec2 center(windowWidth * 0.5f, windowHeight * 0.5f);
                ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::Begin("EliminatedOverlay", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

                ImGui::TextColored(ImVec4(1.0f, 0.15f, 0.20f, 1.0f), "=== PLAYER 456 ELIMINATED ===");
                ImGui::Spacing();
                if (gameState == CAUGHT) {
                    ImGui::Text("You moved while the Doll was scanning during RED LIGHT!");
                } else {
                    ImGui::Text("Time expired before reaching the finish line!");
                }
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00f, 0.08f, 0.48f, 1.0f));
                if (ImGui::Button("TRY AGAIN (R)", ImVec2(-1, 38))) {
                    resetRLGL();
                }
                ImGui::PopStyleColor();

                ImGui::End();
            } else if (gameState == WON) {
                ImVec2 center(windowWidth * 0.5f, windowHeight * 0.5f);
                ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::Begin("WonOverlay", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

                ImGui::TextColored(ImVec4(0.02f, 0.95f, 0.55f, 1.0f), "=== VICTORY! YOU SURVIVED ROUND 1! ===");
                ImGui::Spacing();
                ImGui::Text("Player 456 successfully crossed the finish line!");
                ImGui::Text("Time Remaining: %.2f seconds | Surviving Contestants: %d", gameTimer, aliveCount);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Button to Advance to Round 5 Glass Bridge
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00f, 0.08f, 0.48f, 1.0f));
                if (ImGui::Button("ADVANCE TO ROUND 5: GLASS BRIDGE >>", ImVec2(-1, 42))) {
                    startGlassBridge();
                }
                ImGui::PopStyleColor();
                ImGui::Spacing();

                if (ImGui::Button("PLAY AGAIN (R)", ImVec2(-1, 38))) {
                    resetRLGL();
                }

                ImGui::End();
            }
        } else {
            // Glass Bridge Overlays
            if (gbState == GB_ELIMINATED) {
                ImVec2 center(windowWidth * 0.5f, windowHeight * 0.5f);
                ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::Begin("GBElimOverlay", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

                ImGui::TextColored(ImVec4(1.0f, 0.15f, 0.20f, 1.0f), "=== PLAYER 456 ELIMINATED ===");
                ImGui::Spacing();
                ImGui::Text("You stepped on normal fragile glass and plummeted into the abyss!");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00f, 0.08f, 0.48f, 1.0f));
                if (ImGui::Button("TRY AGAIN (R)", ImVec2(-1, 40))) {
                    resetGlassBridge();
                }
                ImGui::PopStyleColor();

                ImGui::End();
            } else if (gbState == GB_WON) {
                ImVec2 center(windowWidth * 0.5f, windowHeight * 0.5f);
                ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::Begin("GBWonOverlay", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "=== SQUID GAME CHAMPION ===");
                ImGui::TextColored(ImVec4(0.02f, 0.95f, 0.55f, 1.0f), "      PLAYER 456 CONQUERED THE GLASS BRIDGE!");
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "GRAND PRIZE AWARDED: ₩ 45,600,000,000 (₩45.6 BILLION WON)");
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.02f, 0.85f, 0.45f, 1.0f));
                if (ImGui::Button("PLAY AGAIN (R)", ImVec2(-1, 42))) {
                    resetGlassBridge();
                }
                ImGui::PopStyleColor();

                ImGui::End();
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    AudioManager::cleanup();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
