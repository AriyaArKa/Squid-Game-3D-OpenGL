#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <functional>
#include <fstream>

// Modern engine headers
#include "engine/Shader.h"
#include "engine/Camera.h"
#include "engine/GLCompat.h"

// Game subsystems
#include "core/Config.h"
#include "core/Types.h"
#include "audio/Audio.h"
#include "render/Renderer.h"
#include "render/LightFixtures.h"
#include "render/SceneTextures.h"
#include "tools/Showcase.h"
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

    // Shape view (key I): paints every part with the color of its basic shape kind
    // (box, sphere, cylinder, ...) so you can see what each model is built from.
    bool shapeView = false;

    // -----------------------------------------------------------------------------------
    // DIRECTIONAL LIGHT SUN VISUALS (Visible glowing Sun disc & stage floodlight array)
    // -----------------------------------------------------------------------------------
    float sunAzimuth = 45.0f;    // degrees around Y (Upper-Right Sky)
    float sunElevation = 38.0f;  // degrees above ground (directly visible above arena wall)
    // Volumetric helper geometry is optional; the clean cinematic preset starts
    // with it hidden so translucent cones/spheres never wash out the set.
    bool  showSunRays = false;

    void updateSunDirection() {
        LightFixtures::updateSunDirection(sceneLighting.dir, sunElevation, sunAzimuth);
    }

    // ---------------------------------------------------------------------------------
    // REPORT CAPTURE MODE  (game_modern.exe --capture <folder> [name-prefix])
    // Plays a fixed list of "shots" (game moments, camera angles, single models), saves
    // each one as a .bmp and quits. Used to make every screenshot in the report.
    // ---------------------------------------------------------------------------------
    struct CaptureShot {
        std::string name;
        std::function<void()> setup;           // puts the game in the wanted state
        int frames = 90;                       // frames to run before saving (lets cameras settle)
        bool ui = false;                       // keep the HUD in the picture
        std::string showcase;                  // render one model instead of the game
        bool parts = false;                    // showcase: shape-colored exploded view
        std::function<void(int)> onFrame;      // optional per-frame hook (frame number)
    };
    bool captureMode = false;
    std::string captureDir;
    std::vector<CaptureShot> captureShots;
    size_t captureIndex = 0;
    int captureFrame = 0;
    bool captureFreeze = false;      // pause game logic so a posed moment stays still
    bool captureDimAmbient = false;  // light demos: weak ambient so each light's effect shows
    bool captureCamOverride = false; // use captureEye/captureLook instead of the game camera
    glm::vec3 captureEye(0.0f), captureLook(0.0f);
    float captureFov = 50.0f;


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

            // Global Lighting Hotkeys (3 light types) + Shape view
            if (key == GLFW_KEY_I) {
                shapeView = !shapeView;
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
                    case GLFW_KEY_7:
                        sceneLighting.fillEnabled = !sceneLighting.fillEnabled;
                        break;
                    case GLFW_KEY_TAB:
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

    void setupImGuiFonts(ImGuiIO& io) {
        // The built-in ImGui font has no ●, ₩ glyphs (they showed as '?'). Use Windows'
        // Segoe UI Semibold and merge the symbol font for those characters.
        static const ImWchar textRanges[] = { 0x0020, 0x00FF, 0x2000, 0x206F, 0x20A0, 0x20CF, 0 };
        static const ImWchar symbolRanges[] = { 0x20A0, 0x20CF, 0x25A0, 0x25FF, 0 };
        const char* textFont = "C:/Windows/Fonts/seguisb.ttf";
        const char* symbolFont = "C:/Windows/Fonts/seguisym.ttf";
        if (std::ifstream(textFont).good()) {
            io.Fonts->AddFontFromFileTTF(textFont, 16.0f, nullptr, textRanges);
            if (std::ifstream(symbolFont).good()) {
                ImFontConfig merge;
                merge.MergeMode = true;
                io.Fonts->AddFontFromFileTTF(symbolFont, 16.0f, &merge, symbolRanges);
            }
        } else {
            io.Fonts->AddFontDefault();
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

    // ---------------------------------------------------------------------------------
    // REPORT CAPTURE SHOTS
    // ---------------------------------------------------------------------------------
    // Round 1 posed mid-game: player 456 at playerZ, the NPCs spread out ahead of the start.
    void poseRLGL(float playerZ, bool redLight) {
        resetRLGL();
        currentPhase = PHASE_RED_LIGHT_GREEN_LIGHT;
        gameState = PLAYING;
        gameTimer = 41.0f;
        player.z = playerZ;
        player.legAngle = 18.0f;
        player.armAngle = -18.0f;
        for (size_t i = 0; i < npcs.size(); ++i) {
            npcs[i].z = START_Z + 7.0f + fmodf((float)i * 7.3f, 24.0f);
            npcs[i].legAngle = (i % 2) ? 20.0f : -20.0f;
            npcs[i].armAngle = -npcs[i].legAngle;
        }
        if (redLight) {
            doll.lightState = RED_LIGHT;
            doll.headAngle = doll.targetAngle = 180.0f;
            for (auto& n : npcs) n.legAngle = n.armAngle = 0.0f;
            player.legAngle = player.armAngle = 0.0f;
            // two contestants who moved during the red light
            npcs[5].isAlive = false;  npcs[5].fallAngle = 90.0f;
            npcs[20].isAlive = false; npcs[20].fallAngle = 90.0f;
        }
        doll.stateTimer = 1.0e6f;
        captureFreeze = true;
    }

    // Round 2 with Player 456 at `queueNumber`; skipTurns lets the AI play first.
    void poseGB(int queueNumber, bool skipTurns, GlassBridgeCamMode mode) {
        currentPhase = PHASE_GLASS_BRIDGE;
        GlassBridgeManager::init(gbSteps, gbContestants, gbShards, gbPlayerQueueNum, gbActiveQueueIndex,
                                 gbState, gbTimer, queueNumber);
        if (skipTurns) GlassBridgeManager::skipToPlayerTurn(gbSteps, gbContestants, gbActiveQueueIndex, gbState);
        gbTimer = 731.0f;
        gbCamMode = mode;
        gbCamInitialized = false;
        sunAzimuth = 45.0f;
        sunElevation = 50.0f;
        captureFreeze = true;
    }

    void customCamera(glm::vec3 eye, glm::vec3 look, float fov) {
        captureCamOverride = true;
        captureEye = eye;
        captureLook = look;
        captureFov = fov;
    }

    void buildCaptureShots(const std::string& filter) {
        std::vector<CaptureShot> all;
        auto add = [&](const std::string& name, std::function<void()> setup, bool ui = false, int frames = 90) {
            CaptureShot shot;
            shot.name = name;
            shot.setup = setup;
            shot.ui = ui;
            shot.frames = frames;
            all.push_back(shot);
        };
        auto rlglDefaults = [] { sunAzimuth = 45.0f; sunElevation = 38.0f; };

        // ---- Full game screenshots with the HUD
        add("ui_rlgl_start", [=] { rlglDefaults(); resetRLGL(); currentPhase = PHASE_RED_LIGHT_GREEN_LIGHT; switchRLGLCamera(CAM_THIRD_PERSON); }, true);
        add("ui_rlgl_green", [=] { rlglDefaults(); poseRLGL(-36.0f, false); switchRLGLCamera(CAM_THIRD_PERSON); }, true);
        add("ui_rlgl_red", [=] { rlglDefaults(); poseRLGL(-36.0f, true); switchRLGLCamera(CAM_OVERHEAD); }, true);
        add("ui_rlgl_won", [=] { rlglDefaults(); poseRLGL(46.0f, false); gameState = WON; switchRLGLCamera(CAM_FINISH_LINE); }, true);
        add("ui_gb_turn", [] { poseGB(6, true, GB_CAM_FOLLOW); }, true);
        add("ui_gb_won", [] { poseGB(16, true, GB_CAM_DESTINATION);
                              gbContestants[15].hasFinished = true; gbContestants[15].currentStep = GB_NUM_STEPS;
                              gbContestants[15].x = 0; gbContestants[15].z = GB_BRIDGE_START_Z + GB_NUM_STEPS * GB_STEP_LENGTH + 2.5f;
                              gbState = GB_WON;
                              customCamera({4.0f, 19.5f, 49.0f}, {0.0f, 17.2f, 60.0f}, 55.0f); }, true);

        // ---- Round 1 camera modes (no HUD)
        const CameraMode rlglModes[7] = {CAM_THIRD_PERSON, CAM_OVERHEAD, CAM_TOP_DOWN, CAM_DOLL_CLOSEUP,
                                         CAM_SIDE_LEFT, CAM_SIDE_RIGHT, CAM_FINISH_LINE};
        for (int i = 0; i < 7; ++i) {
            CameraMode m = rlglModes[i];
            add("rlgl_cam" + std::to_string(i + 1), [=] { rlglDefaults(); poseRLGL(-36.0f, i == 6); switchRLGLCamera(m); });
        }
        // ---- Round 2 camera modes (no HUD)
        const GlassBridgeCamMode gbModes[5] = {GB_CAM_FOLLOW, GB_CAM_ORBIT_360, GB_CAM_TOP_DOWN, GB_CAM_SIDE_PROFILE, GB_CAM_DESTINATION};
        for (int i = 0; i < 5; ++i) {
            GlassBridgeCamMode m = gbModes[i];
            add("gb_cam" + std::to_string(i + 1), [=] { poseGB(6, true, m); gbOrbitYaw = 35.0f; gbOrbitPitch = 22.0f; gbOrbitDist = 14.0f; });
        }

        // ---- Wide views
        add("rlgl_wide", [=] { rlglDefaults(); poseRLGL(-36.0f, false); customCamera({11.0f, 13.0f, -62.0f}, {-1.0f, 3.0f, 20.0f}, 58.0f); });
        add("gb_hall", [] { poseGB(6, true, GB_CAM_FOLLOW); customCamera({11.0f, 25.0f, -13.0f}, {-1.0f, 14.0f, 24.0f}, 62.0f); });
        add("gb_sides", [] { poseGB(6, true, GB_CAM_FOLLOW); customCamera({0.0f, 21.0f, -3.0f}, {0.0f, 17.5f, 30.0f}, 74.0f); });

        // ---- Glass shattering moment (the player jumps onto the weak pane of step 1)
        {
            CaptureShot shot;
            shot.name = "gb_shatter";
            shot.frames = 34;
            shot.setup = [] {
                poseGB(1, false, GB_CAM_SIDE_PROFILE);
                captureFreeze = false;
                gbContestants[0].z = 0.8f;
                GlassSide weak = (gbSteps[0].safeSide == GLASS_LEFT) ? GLASS_RIGHT : GLASS_LEFT;
                GlassBridgeManager::jumpPlayer(gbSteps, gbContestants, gbShards, weak, 0, gbState);
                customCamera({-7.0f, 17.6f, 0.5f}, {0.6f, 15.6f, 3.2f}, 55.0f);
            };
            all.push_back(shot);
        }

        // ---- The three light types, one at a time (Round 2 and Round 1)
        const char* lightNames[4] = {"dir", "point", "spot", "all"};
        for (int i = 0; i < 4; ++i) {
            add(std::string("light_gb_") + lightNames[i], [=] {
                poseGB(1, false, GB_CAM_FOLLOW);
                gbContestants[0].z = 0.8f;
                sceneLighting.dir.enabled = (i == 0 || i == 3);
                sceneLighting.point.enabled = (i == 1 || i == 3);
                sceneLighting.fillEnabled = (i == 1 || i == 3);
                sceneLighting.spot.enabled = (i == 2 || i == 3);
                captureDimAmbient = true;
                customCamera({7.5f, 22.0f, -10.0f}, {-0.5f, 15.5f, 9.0f}, 58.0f);
            });
            add(std::string("light_rlgl_") + lightNames[i], [=] {
                rlglDefaults();
                poseRLGL(20.0f, true);
                for (auto& n : npcs) n.z += 40.0f;
                sceneLighting.dir.enabled = (i == 0 || i == 3);
                sceneLighting.point.enabled = (i == 1 || i == 3);
                sceneLighting.spot.enabled = (i == 2 || i == 3);
                captureDimAmbient = true;
                customCamera({-17.0f, 13.0f, 4.0f}, {2.0f, 2.5f, 34.0f}, 60.0f);
            });
        }

        // ---- Phong highlight close-up and texture close-ups
        add("phong_doll", [=] { poseRLGL(-36.0f, true); sunAzimuth = 200.0f; sunElevation = 32.0f;
                                customCamera({1.3f, 7.0f, 54.2f}, {0.0f, 6.7f, 58.0f}, 34.0f); });
        add("phong_players", [=] { rlglDefaults(); poseRLGL(-36.0f, false); customCamera({2.6f, 2.2f, -34.0f}, {0.0f, 1.4f, -36.0f}, 42.0f); });
        add("tex_sand", [=] { rlglDefaults(); poseRLGL(-36.0f, false); customCamera({1.0f, 1.8f, -47.0f}, {4.0f, 0.0f, -40.0f}, 55.0f); });
        add("tex_tree", [=] { rlglDefaults(); poseRLGL(-36.0f, false); customCamera({-7.5f, 3.2f, 55.5f}, {0.0f, 3.5f, 60.0f}, 55.0f); });
        add("tex_wall", [] { poseGB(6, true, GB_CAM_FOLLOW); customCamera({-6.0f, 20.0f, 4.0f}, {-20.0f, 21.0f, 16.0f}, 62.0f); });

        // ---- In-game shape view
        add("shapes_rlgl", [=] { rlglDefaults(); poseRLGL(-36.0f, false); shapeView = true; customCamera({11.0f, 13.0f, -62.0f}, {-1.0f, 3.0f, 20.0f}, 58.0f); });
        add("shapes_gb", [] { poseGB(6, true, GB_CAM_FOLLOW); shapeView = true; customCamera({11.0f, 25.0f, -13.0f}, {-1.0f, 14.0f, 24.0f}, 62.0f); });

        // ---- Single models: normal look + split into basic shapes
        for (const std::string& item : Showcase::itemNames()) {
            bool basic = item.rfind("prim_", 0) == 0;
            for (int parts = basic ? 1 : 0; parts <= 1; ++parts) {
                CaptureShot shot;
                shot.name = "obj_" + item + (parts ? "_parts" : "_full");
                shot.showcase = item;
                shot.parts = parts == 1;
                shot.frames = 3;
                shot.setup = [] { poseRLGL(-36.0f, false); };
                all.push_back(shot);
            }
        }

        for (CaptureShot& shot : all) {
            if (filter.empty() || shot.name.rfind(filter, 0) == 0) captureShots.push_back(shot);
        }
    }
}

int main(int argc, char** argv) {
    std::string captureFilter;
    if (argc >= 3 && std::string(argv[1]) == "--capture") {
        captureMode = true;
        captureDir = argv[2];
        if (argc >= 4) captureFilter = argv[3];
        windowWidth = 1600;
        windowHeight = 900;
    }
    srand(captureMode ? 456u : (unsigned int)time(NULL));

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x multisample anti-aliasing (smooth edges)
    if (captureMode) glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight,
                                          "Squid Game 3D: Red Light Green Light & Glass Bridge",
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(captureMode ? 0 : 1); // VSync (off while capturing, to finish faster)

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
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.04f, 0.05f, 0.08f, 1.0f);
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    // Modern Engine Components
    Shader shader("src/shaders/default.vert", "src/shaders/default.frag");
    Camera camera(windowWidth, windowHeight, glm::vec3(0.0f, 2.5f, START_Z - 6.0f));

    // Textures, Audio & Subsystems Initialization
    SceneTextures::loadAll();
    if (!captureMode) AudioManager::init(); // capture runs silently
    EnvironmentManager::init();
    resetRLGL();
    resetGlassBridge();

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = captureMode ? nullptr : "imgui.ini";
    setupSquidGameImGuiStyle();
    setupImGuiFonts(io);
    if (captureMode) buildCaptureShots(captureFilter);
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

        const CaptureShot* shot = nullptr;
        if (captureMode) {
            if (captureIndex >= captureShots.size()) break;
            shot = &captureShots[captureIndex];
            if (captureFrame == 0) {
                // Every shot starts from the same clean defaults, then applies its own setup.
                captureFreeze = false;
                captureCamOverride = false;
                captureDimAmbient = false;
                shapeView = false;
                sceneLighting.dir.enabled = sceneLighting.point.enabled = sceneLighting.fillEnabled = sceneLighting.spot.enabled = true;
                showSunRays = false;
                if (shot->setup) shot->setup();
            }
            if (shot->onFrame) shot->onFrame(captureFrame);
            dt = 1.0f / 60.0f;
            currentTime = captureFrame / 60.0;
        }
        const bool drawUI = !captureMode || shot->ui;

        // -------------------------------------------------------------------------------
        // UPDATE GAME LOGIC PER PHASE
        // -------------------------------------------------------------------------------
        if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
            if (gameState == PLAYING && !captureFreeze) {
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
        } else if (!captureFreeze) {
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
        if (centerW < 300 || !drawUI) {
            leftBarW = 0; rightBarW = 0; centerW = windowWidth;
        }

        // Shape view: color every part by its basic shape kind
        GLCompat::InspectSettings inspectSettings;
        inspectSettings.colorByShape = shapeView;
        GLCompat::setInspect(inspectSettings);

        // Round-specific clear colors keep any uncovered background consistent with
        // the art direction: bright storybook sky versus a deep theatrical chamber.
        if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
            glClearColor(COLOR_RLGL_CLEAR.r, COLOR_RLGL_CLEAR.g, COLOR_RLGL_CLEAR.b, 1.0f);
        } else {
            glClearColor(COLOR_GB_CLEAR.r, COLOR_GB_CLEAR.g, COLOR_GB_CLEAR.b, 1.0f);
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
            if (captureCamOverride) {
                camera.SetView(captureEye, captureLook, glm::vec3(0, 1, 0), captureFov, 0.1f, 280.0f);
            } else {
                camera.SetView(glm::vec3(eyeX, eyeY, eyeZ),
                               glm::vec3(lookX, lookY, lookZ),
                               glm::vec3(upX, upY, upZ),
                               fov, 0.1f, 280.0f);
            }

            // -------------------------------------------------------------
            // DYNAMIC 3-TYPE LIGHTING (RLGL Arena)
            // -------------------------------------------------------------
            // 1. Directional Light: Sun Key Light
            updateSunDirection();
            sceneLighting.dir.color = COLOR_RLGL_SUN;
            sceneLighting.dir.ambientIntensity = 0.42f;
            sceneLighting.dir.diffuseIntensity = 0.82f;
            sceneLighting.dir.specularIntensity = 0.52f;
            sceneLighting.ambientSky = glm::vec3(0.46f, 0.58f, 0.82f);
            sceneLighting.ambientGround = glm::vec3(0.42f, 0.29f, 0.16f);
            sceneLighting.numFill = 0;
            sceneLighting.fogEnabled = true;
            sceneLighting.fogColor = glm::vec3(0.62f, 0.76f, 0.92f); // light sky haze
            sceneLighting.fogDensity = 0.0045f;
            if (captureDimAmbient) sceneLighting.dir.ambientIntensity = 0.12f;

            // 2. Point Light: High Arena Floodlight Tower
            sceneLighting.point.position = glm::vec3(18.0f, 16.0f, 15.0f);
            sceneLighting.point.color = COLOR_RLGL_POINT;
            sceneLighting.point.intensity = 1.15f;

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
            LightFixtures::drawDirectionalSun(sceneLighting.dir, sunElevation, sunAzimuth, showSunRays);
            LightFixtures::drawArenaFloodlight(sceneLighting.point);
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
            if (captureCamOverride) {
                camera.SetView(captureEye, captureLook, glm::vec3(0, 1, 0), captureFov, 0.1f, 300.0f);
            } else {
                camera.SetView(gbSmoothEye,
                               gbSmoothLook,
                               glm::vec3(rawUpX, rawUpY, rawUpZ),
                               rawFov, 0.1f, 300.0f);
            }

            // -------------------------------------------------------------
            // DYNAMIC 3-TYPE LIGHTING (Glass Bridge)
            // -------------------------------------------------------------
            // 1. Directional Light: Stage Overhead Key Light
            updateSunDirection();
            sceneLighting.dir.color = COLOR_GB_FLOOD;
            sceneLighting.dir.ambientIntensity = 0.50f;
            sceneLighting.dir.diffuseIntensity = 0.72f;
            sceneLighting.dir.specularIntensity = 0.60f;
            sceneLighting.ambientSky = glm::vec3(0.66f, 0.70f, 0.84f);  // cool light from the ceiling troughs
            sceneLighting.ambientGround = glm::vec3(0.34f, 0.35f, 0.42f);
            sceneLighting.fogEnabled = true;
            sceneLighting.fogColor = COLOR_GB_FOG;
            sceneLighting.fogDensity = 0.0075f;
            if (captureDimAmbient) sceneLighting.dir.ambientIntensity = 0.12f;

            // Extra point lights: 4 warm lamps on the two side walls light up both sides.
            const float lampZ[2] = {14.0f, 42.0f};
            sceneLighting.numFill = 4;
            for (int i = 0; i < 4; ++i) {
                PointLightData& lamp = sceneLighting.fill[i];
                float side = (i < 2) ? -1.0f : 1.0f;
                lamp.position = glm::vec3(side * 18.4f, GB_BRIDGE_HEIGHT + 7.5f, lampZ[i % 2]);
                lamp.color = COLOR_GB_LAMP;
                lamp.constant = 1.0f;
                lamp.linear = 0.030f;
                lamp.quadratic = 0.0040f;
                lamp.intensity = 1.25f;
            }

            // 2. Point Light: Suspended Glass Bridge Chandelier at bridge center
            sceneLighting.point.position = glm::vec3(0.0f, GB_BRIDGE_HEIGHT + 7.0f, 28.0f);
            sceneLighting.point.color = COLOR_GB_POINT;
            sceneLighting.point.intensity = 1.25f;

            // 3. Spot Light: Theatrical Follow-Spotlight tracking active jumper
            glm::vec3 activePos(0.0f, GB_BRIDGE_HEIGHT, 0.0f);
            if (gbActiveQueueIndex < (int)gbContestants.size()) {
                activePos = glm::vec3(gbContestants[gbActiveQueueIndex].x,
                                      gbContestants[gbActiveQueueIndex].y,
                                      gbContestants[gbActiveQueueIndex].z);
            }
            sceneLighting.spot.position = glm::vec3(activePos.x, GB_BRIDGE_HEIGHT + 8.5f, activePos.z);
            sceneLighting.spot.direction = glm::vec3(0.0f, -1.0f, 0.0f);
            sceneLighting.spot.color = COLOR_GB_SPOT; // Crisp theatrical follow beam
            sceneLighting.spot.intensity = 2.35f;

            GLCompat::beginFrame(shader, camera, sceneLighting);

            GlassBridgeManager::drawEnvironment();
            GlassBridgeManager::drawBridge(gbSteps, (float)currentTime, gbContestants, gbActiveQueueIndex);
            GlassBridgeManager::drawContestants(gbContestants);
            GlassBridgeManager::drawShards(gbShards);

            // Draw physical light fixtures: Overhead Stage Floodlights, Chandelier & Follow-Spotlight
            LightFixtures::drawOverheadFloodlightArray(sceneLighting.dir, sunElevation, sunAzimuth, showSunRays);
            LightFixtures::drawBridgeChandelier(sceneLighting.point);
            LightFixtures::drawFollowSpotlight(sceneLighting.spot, showSunRays);
            for (int i = 0; i < sceneLighting.numFill; ++i) {
                LightFixtures::drawWallLamp(sceneLighting.fill[i], sceneLighting.fillEnabled,
                                            sceneLighting.fill[i].position.x < 0.0f ? -1.0f : 1.0f);
            }
        }

        GLCompat::setInspect(GLCompat::InspectSettings());

        // Showcase shot: one model alone on a light background (report figures)
        if (shot && !shot->showcase.empty()) {
            const int side = windowHeight;
            glEnable(GL_SCISSOR_TEST);
            glScissor(0, 0, side, side);
            glClearColor(0.955f, 0.960f, 0.970f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_SCISSOR_TEST);
            std::string counts = Showcase::render(shot->showcase, shot->parts, shader, camera, 0, 0, side, side);
            if (captureFrame + 1 >= shot->frames) {
                std::ofstream(captureDir + "/" + shot->name + ".txt") << counts;
            }
        }

        // =========================================================================
        // PASS 3: MODERN DEAR IMGUI HUD & INTERACTIVE CONTROLS
        // =========================================================================
        glViewport(0, 0, windowWidth, windowHeight);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        bool pipRectVisible = false;
        int pipRectX = 0, pipRectY = 0, pipRectW = 0, pipRectH = 0;
        if (drawUI) {

        // 0. TOP PHASE NAVIGATOR & SQUID GAME BROADCAST HEADER (Glass Bridge Phase Only)
        if (currentPhase == PHASE_GLASS_BRIDGE) {
            ImGui::SetNextWindowPos(ImVec2((float)leftBarW + 12.0f, 6.0f));
            ImGui::SetNextWindowSize(ImVec2((float)centerW - 24.0f, 76.0f));
            ImGui::SetNextWindowBgAlpha(0.78f); // dark strip keeps the header readable on the light hall
            ImGui::Begin("TopNavBar", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

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
                pipRectVisible = true;
                pipRectX = (int)pipMin.x;
                pipRectY = windowHeight - (int)pipMax.y; // ImGui y grows down, OpenGL y grows up
                pipRectW = pipW;
                pipRectH = pipH;
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
                    normX = glm::clamp(normX, 0.0f, 1.0f);
                    normZ = glm::clamp(normZ, 0.0f, 1.0f);
                    float dotX = radarPos.x + 10.0f + normX * (radarW - 20.0f);
                    float dotY = radarPos.y + radarH - 12.0f - normZ * (radarH - 24.0f);
                    dl->AddCircleFilled(ImVec2(dotX, dotY), 2.5f, IM_COL32(0, 235, 120, 220));
                }

                // Draw Player 456 on radar
                {
                    float normX = (player.x + ARENA_HALF_WIDTH) / (ARENA_HALF_WIDTH * 2.0f);
                    float normZ = (player.z - START_Z) / totalDist;
                    normX = glm::clamp(normX, 0.0f, 1.0f);
                    normZ = glm::clamp(normZ, 0.0f, 1.0f);
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
                    if (ImGui::Button(">> FAST-FORWARD: ON (TAB)", ImVec2(-1, 34))) {
                        gbFastForward = false;
                    }
                    ImGui::PopStyleColor(2);
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.20f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.80f, 1.0f, 0.8f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
                    if (ImGui::Button(">> FAST-FORWARD AI (TAB)", ImVec2(-1, 34))) {
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
                if (ImGui::BeginChild("MoveCard", ImVec2(-1, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY)) {
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
                if (ImGui::BeginChild("SpeedCard", ImVec2(-1, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY)) {
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
                if (ImGui::BeginChild("SystemCard", ImVec2(-1, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY)) {
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
                if (ImGui::BeginChild("ActionCard", ImVec2(-1, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY)) {
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
                if (ImGui::BeginChild("SystemCard", ImVec2(-1, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY)) {
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

            if (ImGui::BeginChild("LightingCard", ImVec2(-1, 0), true)) {
                ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "LIGHTING & SHADING");
                if (currentPhase != PHASE_RED_LIGHT_GREEN_LIGHT) {
                    ImGui::TextColored(ImVec4(0.20f, 1.0f, 0.60f, 1.0f), "Phong shading (per pixel)");
                }
                ImGui::Spacing();

                // Shape view: shows which basic shapes each model is made of
                if (ImGui::Checkbox("Shape view [I]", &shapeView)) {}
                if (shapeView) {
                    const GLCompat::ShapeKind kinds[] = {
                        GLCompat::SHAPE_BOX, GLCompat::SHAPE_SPHERE, GLCompat::SHAPE_CYLINDER,
                        GLCompat::SHAPE_CONE, GLCompat::SHAPE_DISK, GLCompat::SHAPE_TORUS,
                        GLCompat::SHAPE_POLYGON
                    };
                    for (GLCompat::ShapeKind k : kinds) {
                        glm::vec3 c = GLCompat::shapeColor(k);
                        ImGui::TextColored(ImVec4(c.r, c.g, c.b, 1.0f), "  %s", GLCompat::shapeName(k));
                    }
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

                // (b) Point Lights & Wall Lamps & Spot Light
                if (currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
                    ImGui::Checkbox("2. Tower Point Light [8]", &sceneLighting.point.enabled);
                    ImGui::Checkbox("3. Doll Eye Spot [9]", &sceneLighting.spot.enabled);
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.70f, 0.70f, 0.75f, 1.0f), "Hotkeys: 0/8/9 lights | I shapes");
                } else {
                    ImGui::Checkbox("2. Chandelier [8]", &sceneLighting.point.enabled);
                    ImGui::Checkbox("3. Wall Lamps (4x) [7]", &sceneLighting.fillEnabled);
                    ImGui::Checkbox("4. Jumper Spot [9]", &sceneLighting.spot.enabled);
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.70f, 0.70f, 0.75f, 1.0f), "Hotkeys: 0/8/7/9 lights | I shapes");
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            ImGui::End();
        }

        // 2b. FLOATING SHADING MODE HUD BADGE (Bottom right of 3D Scene Viewport)
        if (shapeView || currentPhase != PHASE_RED_LIGHT_GREEN_LIGHT) {
            float badgeX = (float)(windowWidth - rightBarW - 180);
            ImGui::SetNextWindowPos(ImVec2(badgeX, (float)windowHeight - 44.0f));
            ImGui::SetNextWindowSize(ImVec2(170.0f, 34.0f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.06f, 0.10f, 0.88f));
            ImGui::PushStyleColor(ImGuiCol_Border, shapeView ? ImVec4(0.22f, 0.50f, 0.95f, 0.9f) : ImVec4(0.10f, 0.90f, 0.50f, 0.9f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
            ImGui::Begin("ShadingBadge", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
            if (shapeView) {
                ImGui::TextColored(ImVec4(0.45f, 0.70f, 1.0f, 1.0f), "SHAPE VIEW  [I]");
            } else {
                ImGui::TextColored(ImVec4(0.20f, 1.0f, 0.60f, 1.0f), "PHONG SHADING");
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
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

        } // drawUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // =========================================================================
        // PASS 3: CCTV DOLL CAM (picture-in-picture, RLGL only)
        // Drawn after the HUD into the frame the left panel reserved for it, so the
        // panel's background can't cover it.
        // =========================================================================
        if (pipRectVisible && currentPhase == PHASE_RED_LIGHT_GREEN_LIGHT) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(pipRectX, pipRectY, pipRectW, pipRectH);
            glViewport(pipRectX, pipRectY, pipRectW, pipRectH);
            glClearColor(COLOR_RLGL_CLEAR.r, COLOR_RLGL_CLEAR.g, COLOR_RLGL_CLEAR.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            CameraManager::getDollCloseupVectors(doll, eyeX, eyeY, eyeZ, lookX, lookY, lookZ, upX, upY, upZ, fov);
            camera.width = pipRectW;
            camera.height = pipRectH;
            camera.SetView(glm::vec3(eyeX, eyeY, eyeZ),
                           glm::vec3(lookX, lookY, lookZ),
                           glm::vec3(upX, upY, upZ),
                           fov, 0.05f, 260.0f);

            GLCompat::setInspect(inspectSettings);
            GLCompat::beginFrame(shader, camera, sceneLighting);
            EnvironmentManager::draw(gameTimer);
            DollManager::draw(doll);
            GuardsManager::draw(guards);
            ContestantsManager::drawNPCs(npcs);
            ContestantsManager::drawPlayer(player);
            ContestantsManager::drawParticles(particles);
            GLCompat::setInspect(GLCompat::InspectSettings());

            glDisable(GL_SCISSOR_TEST);
            glViewport(0, 0, windowWidth, windowHeight);
        }

        if (captureMode) {
            if (++captureFrame >= shot->frames) {
                int w = shot->showcase.empty() ? windowWidth : windowHeight;
                std::string path = captureDir + "/" + shot->name + ".bmp";
                if (Capture::saveBMP(path, 0, 0, w, windowHeight)) std::cout << "saved " << path << std::endl;
                captureFrame = 0;
                captureIndex++;
            }
        }

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
