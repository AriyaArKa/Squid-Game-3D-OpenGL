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

// Dear ImGui modern UI
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace {
    Player player;
    Doll doll;
    std::vector<Guard> guards;
    std::vector<NPCContestant> npcs;
    std::vector<Particle> particles;

    GameState gameState = WAITING;
    CameraMode camMode = CAM_THIRD_PERSON;

    float gameTimer = GAME_TIME_LIMIT;
    int aliveCount = TOTAL_CONTESTANTS;

    int windowWidth = 1100;
    int windowHeight = 720;

    // Movement key states
    bool keyForward = false;
    bool keyBackward = false;
    bool keyTurnLeft = false;
    bool keyTurnRight = false;
    bool keyStrafeLeft = false;
    bool keyStrafeRight = false;
    bool keyHoldSprint = false;
    bool keyHoldWalk   = false;

    // Mouse drag tracking for 360 orbit
    bool isMouseDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    void startGame() {
        if (gameState == WAITING) {
            gameState = PLAYING;
            AudioManager::restartSong();
        }
    }

    void resetGame() {
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

        // Win condition: crossed finish line
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

    void switchCamera(CameraMode newMode) {
        camMode = newMode;
        CameraManager::resetAnglesForMode(camMode);
    }

    // GLFW Callbacks
    void framebufferSizeCallback(GLFWwindow*, int w, int h) {
        windowWidth = (w > 0) ? w : 1;
        windowHeight = (h > 0) ? h : 1;
    }

    void scrollCallback(GLFWwindow*, double, double yoffset) {
        CameraManager::zoom((float)(-yoffset * 0.9), camMode);
    }

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return;

        if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                if (gameState == WAITING) {
                    startGame();
                }
                isMouseDragging = true;
                glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
                CameraManager::handleMouseClick(0, 0, (int)lastMouseX, (int)lastMouseY);
            } else if (action == GLFW_RELEASE) {
                isMouseDragging = false;
                CameraManager::handleMouseClick(0, 1, (int)lastMouseX, (int)lastMouseY);
            }
        }
    }

    void cursorPosCallback(GLFWwindow*, double xpos, double ypos) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return;

        if (isMouseDragging) {
            CameraManager::handleMouseMotion((int)xpos, (int)ypos);
        }
    }

    void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) return;

        bool isPress = (action == GLFW_PRESS || action == GLFW_REPEAT);
        keyHoldSprint = (mods & GLFW_MOD_SHIFT) != 0;
        keyHoldWalk   = (mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) != 0;

        if (action == GLFW_PRESS && gameState == WAITING) {
            if (key == GLFW_KEY_SPACE || key == GLFW_KEY_ENTER || key == GLFW_KEY_W || key == GLFW_KEY_S) {
                startGame();
            }
        }

        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(window, true);
                    break;
                case GLFW_KEY_R:
                    resetGame();
                    break;
                case GLFW_KEY_SPACE:
                    if (gameState == WAITING) startGame();
                    break;
                case GLFW_KEY_Z:
                    player.speedTier = SPEED_WALK;
                    break;
                case GLFW_KEY_X:
                    player.speedTier = SPEED_JOG;
                    break;
                case GLFW_KEY_C:
                    player.speedTier = SPEED_SPRINT;
                    break;
                case GLFW_KEY_G:
                    if (player.speedTier == SPEED_WALK) player.speedTier = SPEED_JOG;
                    else if (player.speedTier == SPEED_JOG) player.speedTier = SPEED_SPRINT;
                    else player.speedTier = SPEED_WALK;
                    break;
                case GLFW_KEY_1: switchCamera(CAM_THIRD_PERSON); break;
                case GLFW_KEY_2: switchCamera(CAM_OVERHEAD); break;     // Arena Free 360° Cam
                case GLFW_KEY_3: switchCamera(CAM_TOP_DOWN); break;     // Aerial Top View
                case GLFW_KEY_4: switchCamera(CAM_DOLL_CLOSEUP); break;
                case GLFW_KEY_5: switchCamera(CAM_SIDE_LEFT); break;
                case GLFW_KEY_6: switchCamera(CAM_SIDE_RIGHT); break;
                case GLFW_KEY_7: switchCamera(CAM_FINISH_LINE); break;  // Finish Line 360° Cam
                case GLFW_KEY_V: case GLFW_KEY_N:
                    CameraManager::cycleMode(camMode);
                    break;
                case GLFW_KEY_F:
                    Renderer::toggleShading();
                    break;
            }
        }

        // Movement updates
        switch (key) {
            case GLFW_KEY_W: case GLFW_KEY_UP:    keyForward = isPress; break;
            case GLFW_KEY_S: case GLFW_KEY_DOWN:  keyBackward = isPress; break;
            case GLFW_KEY_A: case GLFW_KEY_LEFT:  keyTurnLeft = isPress; break;
            case GLFW_KEY_D: case GLFW_KEY_RIGHT: keyTurnRight = isPress; break;
            case GLFW_KEY_Q: keyStrafeLeft = isPress; break;
            case GLFW_KEY_E: keyStrafeRight = isPress; break;
        }
    }

    void setupSquidGameImGuiStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.PopupRounding = 6.0f;
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg]             = ImVec4(0.07f, 0.09f, 0.12f, 0.88f);
        colors[ImGuiCol_Border]               = ImVec4(0.91f, 0.17f, 0.49f, 0.65f); // Hot pink border
        colors[ImGuiCol_FrameBg]              = ImVec4(0.12f, 0.15f, 0.20f, 0.75f);
        colors[ImGuiCol_TitleBg]              = ImVec4(0.08f, 0.10f, 0.14f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.91f, 0.17f, 0.49f, 0.85f);
        colors[ImGuiCol_Button]               = ImVec4(0.85f, 0.15f, 0.45f, 0.70f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.95f, 0.22f, 0.55f, 0.90f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(1.00f, 0.30f, 0.65f, 1.00f);
        colors[ImGuiCol_Header]               = ImVec4(0.18f, 0.24f, 0.32f, 0.80f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.91f, 0.17f, 0.49f, 0.60f);
        colors[ImGuiCol_HeaderActive]         = ImVec4(0.91f, 0.17f, 0.49f, 0.90f);
        colors[ImGuiCol_Text]                 = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
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
                                          "Squid Game 3D: Red Light, Green Light (Modern OpenGL 3.3 Core)",
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync

    // Set GLFW Callbacks
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
    glClearColor(0.10f, 0.25f, 0.55f, 1.0f); // Deep rich atmospheric sky tone

    // Modern Engine Components
    Shader shader("src/shaders/default.vert", "src/shaders/default.frag");
    Camera camera(windowWidth, windowHeight, glm::vec3(0.0f, 2.5f, START_Z - 6.0f));

    // Audio & Subsystems
    AudioManager::init();
    EnvironmentManager::init();
    resetGame();

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    setupSquidGameImGuiStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    double lastTime = glfwGetTime();

    // Main Game Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double currentTime = glfwGetTime();
        float dt = (float)(currentTime - lastTime);
        lastTime = currentTime;
        if (dt > 0.05f) dt = 0.05f; // Clamp delta time to prevent spiral

        // Update Game Logic
        if (gameState == PLAYING) {
            gameTimer -= dt;
            if (gameTimer <= 0.0f) {
                gameTimer = 0.0f;
                checkPlayerElimination(false);
            }

            DollManager::update(doll, dt);
            GuardsManager::update(guards, dt);
            ContestantsManager::update(player, npcs, guards, doll, particles, aliveCount, dt);

            // Player Physics & Movement
            if (player.isAlive) {
                if (keyTurnLeft)  player.facingAngle += TURN_SPEED;
                if (keyTurnRight) player.facingAngle -= TURN_SPEED;

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

                // Arena boundaries
                if (player.x < -ARENA_HALF_WIDTH + 0.8f) player.x = -ARENA_HALF_WIDTH + 0.8f;
                if (player.x >  ARENA_HALF_WIDTH - 0.8f) player.x =  ARENA_HALF_WIDTH - 0.8f;
                if (player.z < -ARENA_HALF_LENGTH + 0.8f) player.z = -ARENA_HALF_LENGTH + 0.8f;

                // Walk cycle animation
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

        // =========================================================================
        // PASS 1: MAIN 3D ARENA VIEWPORT (Center of screen between sidebars)
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
        CameraManager::getViewVectors(player, doll, camMode,
                                      eyeX, eyeY, eyeZ, lookX, lookY, lookZ, upX, upY, upZ, fov);
        camera.width = centerW;
        camera.height = windowHeight;
        camera.SetView(glm::vec3(eyeX, eyeY, eyeZ),
                       glm::vec3(lookX, lookY, lookZ),
                       glm::vec3(upX, upY, upZ),
                       fov, 0.1f, 280.0f);

        glm::vec3 lightDir(-0.4f, -1.0f, -0.3f);
        glm::vec3 lightColor(0.75f, 0.72f, 0.68f);

        GLCompat::beginFrame(shader, camera, lightDir, lightColor);

        // Render full 3D game world
        EnvironmentManager::draw(gameTimer);
        DollManager::draw(doll);
        GuardsManager::draw(guards);
        ContestantsManager::drawNPCs(npcs);
        ContestantsManager::drawPlayer(player);
        ContestantsManager::drawParticles(particles);

        // =========================================================================
        // PASS 2: CORNER CCTV SURVEILLANCE PIP (CAM 01 - Doll Closeup)
        // =========================================================================
        if (leftBarW > 100) {
            int pipPad = 12;
            int pipW = leftBarW - pipPad * 2;
            int pipH = (int)(pipW * 0.85f);
            int pipX = pipPad;
            int pipY = windowHeight - pipH - 68;

            glEnable(GL_SCISSOR_TEST);
            glScissor(pipX, pipY, pipW, pipH);
            glViewport(pipX, pipY, pipW, pipH);
            glClearColor(0.26f, 0.52f, 0.82f, 1.0f); // Bright blue soundstage sky background
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            CameraManager::getDollCloseupVectors(doll, eyeX, eyeY, eyeZ, lookX, lookY, lookZ, upX, upY, upZ, fov);
            camera.width = pipW;
            camera.height = pipH;
            camera.SetView(glm::vec3(eyeX, eyeY, eyeZ),
                           glm::vec3(lookX, lookY, lookZ),
                           glm::vec3(upX, upY, upZ),
                           fov, 0.1f, 80.0f);

            // Frontal camera headlight: lightDir is ray direction from camera towards doll
            glm::vec3 camDir = glm::normalize(glm::vec3(lookX - eyeX, lookY - eyeY, lookZ - eyeZ));
            glm::vec3 pipLightDir = camDir; // Ray points from camera to doll
            glm::vec3 pipLightColor(1.40f, 1.35f, 1.30f); // Bright vivid CCTV fill light!

            GLCompat::beginFrame(shader, camera, pipLightDir, pipLightColor);
            DollManager::draw(doll);
            GuardsManager::draw(guards);

            glDisable(GL_SCISSOR_TEST);
            glClearColor(0.08f, 0.22f, 0.52f, 1.0f);
        }

        // =========================================================================
        // PASS 3: MODERN DEAR IMGUI HUD & CONTROL OVERLAYS
        // =========================================================================
        glViewport(0, 0, windowWidth, windowHeight);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

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

            // Title & Subtitle Header
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "SQUID GAME");
            ImGui::TextColored(ImVec4(0.70f, 0.70f, 0.78f, 1.0f), "RED LIGHT, GREEN LIGHT");
            ImGui::Spacing();

            // Doll Cam Header with State Indicator
            if (doll.lightState == RED_LIGHT) {
                ImGui::TextColored(ImVec4(1.0f, 0.15f, 0.15f, 1.0f), "● SCANNING");
            } else {
                ImGui::TextColored(ImVec4(0.15f, 0.95f, 0.35f, 1.0f), "● CHANTING");
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

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f, 0.04f, 0.06f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.85f, 0.95f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            if (ImGui::BeginChild("ClockBox", ImVec2(-1, 46), true)) {
                ImGui::SetCursorPosY(12.0f);
                ImGui::TextColored(ImVec4(1.0f, 0.15f, 0.15f, 1.0f), "%s", clockBuf);
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            ImGui::Spacing();

            // Pink Bordered Players Count Box
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.94f, 0.16f, 0.48f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            if (ImGui::BeginChild("PlayerBox", ImVec2(-1, 38), true)) {
                ImGui::SetCursorPosY(8.0f);
                ImGui::TextColored(ImVec4(1.0f, 0.92f, 0.20f, 1.0f), "PLAYERS: %d / %d", aliveCount, TOTAL_CONTESTANTS);
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            ImGui::Spacing();

            // Green / Red Light Status Banner
            if (gameState == WAITING || doll.lightState == GREEN_LIGHT) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.75f, 0.28f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.85f, 0.35f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.75f, 0.28f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::Button("GREEN LIGHT - MOVE!", ImVec2(-1, 36));
                ImGui::PopStyleColor(4);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.08f, 0.08f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.15f, 0.15f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.08f, 0.08f, 1.0f));
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

            ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.90f, 1.0f), "COURSE PROGRESS");
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.94f, 0.16f, 0.48f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
            ImGui::ProgressBar(progress, ImVec2(-1, 14), "");
            ImGui::PopStyleColor(2);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d%% TO FINISH", (int)(progress * 100.0f));
            ImGui::Spacing();

            // Speed Pace Badge
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 0.90f));
            if (player.speedTier == SPEED_WALK) {
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.85f, 1.0f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("PaceBox", ImVec2(-1, 32), true)) {
                    ImGui::SetCursorPosY(6.0f);
                    ImGui::TextColored(ImVec4(0.3f, 0.9f, 1.0f, 1.0f), "PACE: WALK (Z)");
                }
            } else if (player.speedTier == SPEED_SPRINT) {
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.35f, 0.15f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("PaceBox", ImVec2(-1, 32), true)) {
                    ImGui::SetCursorPosY(6.0f);
                    ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.2f, 1.0f), "PACE: SPRINT (C)");
                }
            } else {
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.95f, 0.35f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
                if (ImGui::BeginChild("PaceBox", ImVec2(-1, 32), true)) {
                    ImGui::SetCursorPosY(6.0f);
                    ImGui::TextColored(ImVec4(0.15f, 0.95f, 0.35f, 1.0f), "PACE: JOG (X)");
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            ImGui::End();
        }

        // 2. RIGHT SIDEBAR PANEL (Simplified, High-Contrast with Active Highlighting)
        {
            ImGui::SetNextWindowPos(ImVec2((float)(windowWidth - rightBarW), 0));
            ImGui::SetNextWindowSize(ImVec2((float)rightBarW, (float)windowHeight));
            ImGui::Begin("RightPanel", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);

            ImGui::TextColored(ImVec4(1.0f, 0.92f, 0.25f, 1.0f), "GAME CONTROLS");
            ImGui::Separator();
            ImGui::Spacing();

            // CARD 1: CAMERA ANGLES (5 Modes, highlighted active)
            ImGui::TextColored(ImVec4(0.94f, 0.30f, 0.65f, 1.0f), "CAMERA ANGLES (KEY)");
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
                    // Solid bright green button with white text (exact match for GREEN LIGHT - MOVE!)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.75f, 0.28f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.85f, 0.35f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.75f, 0.28f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    if (ImGui::Button(camItems[i].label, ImVec2(-1, 26))) {
                        switchCamera(camItems[i].mode);
                    }
                    ImGui::PopStyleColor(4);
                } else {
                    // Dark card with pink border and gold text (exact match for PLAYERS: 456 / 456)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.06f, 0.08f, 0.94f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.10f, 0.15f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.94f, 0.16f, 0.48f, 0.4f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.94f, 0.16f, 0.48f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.92f, 0.20f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
                    const char* inactiveLabel = camItems[i].label + 3; // Strip ">> "
                    if (ImGui::Button(inactiveLabel, ImVec2(-1, 26))) {
                        switchCamera(camItems[i].mode);
                    }
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(5);
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // CARD 2: SPEED & MOVEMENT (Cyan border card)
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 0.94f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.15f, 0.85f, 0.95f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.8f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            if (ImGui::BeginChild("MoveCard", ImVec2(-1, 52), true)) {
                ImGui::TextColored(ImVec4(0.20f, 0.90f, 1.0f, 1.0f), "W / S : FORWARD / BACK");
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "A / D : TURN LEFT / RIGHT");
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            ImGui::Spacing();

            // CARD 3: SPEED TIERS (Green border card)
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 0.94f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.25f, 1.0f, 0.45f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.8f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            if (ImGui::BeginChild("SpeedCard", ImVec2(-1, 52), true)) {
                ImGui::TextColored(ImVec4(0.30f, 1.0f, 0.50f, 1.0f), "Z/X/C : WALK / JOG / SPRINT");
                ImGui::TextColored(ImVec4(1.0f, 0.92f, 0.20f, 1.0f), "G     : CYCLE SPEED GEAR");
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);
            ImGui::Spacing();

            // CARD 4: SYSTEM CONTROLS (Pink border card)
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 0.94f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.94f, 0.16f, 0.48f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.8f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            if (ImGui::BeginChild("SystemCard", ImVec2(-1, 68), true)) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "MOUSE DRAG  : 360° ORBIT");
                ImGui::TextColored(ImVec4(0.20f, 0.90f, 1.0f, 1.0f), "MOUSE WHEEL : ZOOM IN / OUT");
                ImGui::TextColored(ImVec4(1.0f, 0.92f, 0.20f, 1.0f), "V : CYCLE VIEWS | R : RESTART");
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            ImGui::End();
        }

        // 3. CENTER SCREEN STATE OVERLAYS (Waiting / Eliminated / Won)
        if (gameState == WAITING) {
            ImVec2 center(windowWidth * 0.5f, windowHeight * 0.5f);
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("StartOverlay", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

            ImGui::TextColored(ImVec4(0.95f, 0.25f, 0.55f, 1.0f), "=== SQUID GAME: RED LIGHT, GREEN LIGHT ===");
            ImGui::Spacing();
            ImGui::Text("Advance to the Pink Finish Line before the 60s timer expires.");
            ImGui::Text("Only move when the Doll is facing the tree (Green Light).");
            ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "FREEZE IMMEDIATELY when she turns her head (Red Light)!");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("START GAME (SPACE / CLICK)", ImVec2(-1, 40))) {
                startGame();
            }

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

            if (ImGui::Button("TRY AGAIN (R)", ImVec2(-1, 38))) {
                resetGame();
            }

            ImGui::End();
        } else if (gameState == WON) {
            ImVec2 center(windowWidth * 0.5f, windowHeight * 0.5f);
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("WonOverlay", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

            ImGui::TextColored(ImVec4(0.2f, 0.95f, 0.45f, 1.0f), "=== VICTORY! YOU SURVIVED! ===");
            ImGui::Spacing();
            ImGui::Text("Player 456 successfully crossed the finish line!");
            ImGui::Text("Time Remaining: %.2f seconds | Surviving Contestants: %d", gameTimer, aliveCount);
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("PLAY AGAIN (R)", ImVec2(-1, 38))) {
                resetGame();
            }

            ImGui::End();
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
