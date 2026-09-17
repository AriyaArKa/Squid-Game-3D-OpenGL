#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "Config.h"
#include "Types.h"
#include "Audio.h"
#include "Renderer.h"
#include "Doll.h"
#include "Guards.h"
#include "Contestants.h"
#include "Environment.h"
#include "UI.h"
#include "Camera.h"

// -----------------------------------------------------------------------------------
// GLOBAL GAME INSTANCES & STATE
// -----------------------------------------------------------------------------------
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

    // Movement key flags
    bool keyForward = false;
    bool keyBackward = false;
    bool keyTurnLeft = false;
    bool keyTurnRight = false;
    bool keyStrafeLeft = false;
    bool keyStrafeRight = false;

    // Speed modifier keys
    bool keyHoldSprint = false; // Shift
    bool keyHoldWalk   = false; // Ctrl or Alt
}

// -----------------------------------------------------------------------------------
// GAME STATE MANAGEMENT
// -----------------------------------------------------------------------------------
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

    // Win condition: reached or crossed finish line
    if (player.z >= FINISH_Z) {
        gameState = WON;
        AudioManager::stopAll();
        AudioManager::playWinMusic();
        return;
    }

    // Time out condition
    if (gameTimer <= 0.0f) {
        gameState = TIME_UP;
        player.isAlive = false;
        aliveCount--;
        GuardsManager::triggerShot(guards, player.x, player.z);
        ContestantsManager::spawnBloodParticles(particles, player.x, player.y, player.z);
        AudioManager::playGameOverMusic();
        return;
    }

    // Red light movement check (doll's head has turned and scanning)
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

// -----------------------------------------------------------------------------------
// GLUT CALLBACKS
// -----------------------------------------------------------------------------------
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ==========================================
    // 1. MAIN 3D ARENA VIEWPORT (Full Screen)
    // ==========================================
    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)windowWidth / (float)(windowHeight ? windowHeight : 1);
    gluPerspective(55.0, aspect, 0.1, 280.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Setup Main Camera view (Supports 360° doll/player/overhead orbit)
    CameraManager::applyView(player, doll, camMode);

    // World lighting
    Renderer::setupLighting();

    // Draw Main Scene
    EnvironmentManager::draw(gameTimer);
    DollManager::draw(doll);
    GuardsManager::draw(guards);
    ContestantsManager::drawNPCs(npcs);
    ContestantsManager::drawPlayer(player);
    ContestantsManager::drawParticles(particles);

    // ==========================================
    // 2. CORNER PiP DOLL SURVEILLANCE CAM
    // ==========================================
    int pipW = (int)(windowWidth * 0.22f);
    if (pipW < 230) pipW = 230;
    if (pipW > 340) pipW = 340;
    int pipH = (int)(pipW * 0.72f);
    int pipX = 20;
    int pipY = windowHeight - pipH - 75;

    glEnable(GL_SCISSOR_TEST);
    glScissor(pipX, pipY, pipW, pipH);
    glViewport(pipX, pipY, pipW, pipH);
    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(42.0, (float)pipW / (float)pipH, 0.1, 80.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Close-up camera pointing directly at Doll's face
    CameraManager::applyDollCloseupView(doll);
    Renderer::setupLighting();

    DollManager::draw(doll);
    GuardsManager::draw(guards);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDisable(GL_SCISSOR_TEST);

    // ==========================================
    // 3. 2D HUD OVERLAYS & COUNTERS
    // ==========================================
    glViewport(0, 0, windowWidth, windowHeight);

    // Draw PiP CCTV frame border
    UIManager::drawDollCamFrame((float)pipX, (float)pipY, (float)pipW, (float)pipH, gameTimer);

    // Render Main HUD (05:00 Digital Clock, Player Count, Speed Badge, Status Banner)
    UIManager::renderHUD(windowWidth, windowHeight, gameState, doll.lightState,
                         gameTimer, aliveCount, player, camMode);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)w / (float)h;
    gluPerspective(55.0, aspect, 0.1, 280.0);
    glMatrixMode(GL_MODELVIEW);
}

void mouse(int button, int state, int x, int y) {
    if (gameState == WAITING && state == GLUT_DOWN) {
        startGame();
    }
    
    // Mouse wheel zoom handling
    if (button == 3) { // Wheel up -> Zoom In
        CameraManager::zoom(-0.9f, camMode);
    } else if (button == 4) { // Wheel down -> Zoom Out
        CameraManager::zoom(0.9f, camMode);
    } else {
        CameraManager::handleMouseClick(button, state, x, y);
    }
}

void motion(int x, int y) {
    CameraManager::handleMouseMotion(x, y);
}

void switchCamera(CameraMode newMode) {
    camMode = newMode;
    CameraManager::resetAnglesForMode(camMode);
}

void keyboard(unsigned char key, int x, int y) {
    if (gameState == WAITING) {
        if (key == ' ' || key == 13 || key == 'w' || key == 'W' || key == 's' || key == 'S') {
            startGame();
            return;
        }
    }

    int modifiers = glutGetModifiers();
    keyHoldSprint = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
    keyHoldWalk   = (modifiers & (GLUT_ACTIVE_CTRL | GLUT_ACTIVE_ALT)) != 0;

    switch (key) {
        case 'w': case 'W': keyForward = true; break;
        case 's': case 'S': keyBackward = true; break;
        case 'a': case 'A': keyTurnLeft = true; break;
        case 'd': case 'D': keyTurnRight = true; break;
        case 'q': case 'Q': keyStrafeLeft = true; break;
        case 'e': case 'E': keyStrafeRight = true; break;

        // Multi-Speed Controls:
        case 'z': case 'Z': // Set Slow Walk
            player.speedTier = SPEED_WALK;
            break;
        case 'x': case 'X': // Set Easy Run / Jog
            player.speedTier = SPEED_JOG;
            break;
        case 'c': case 'C': // Set Fast Sprint
            player.speedTier = SPEED_SPRINT;
            break;
        case 'g': case 'G': // Cycle Speed Gear
            if (player.speedTier == SPEED_WALK) player.speedTier = SPEED_JOG;
            else if (player.speedTier == SPEED_JOG) player.speedTier = SPEED_SPRINT;
            else player.speedTier = SPEED_WALK;
            break;

        // Camera Hotkeys:
        case '1': switchCamera(CAM_THIRD_PERSON); break;
        case '2': switchCamera(CAM_FIRST_PERSON); break;
        case '3': switchCamera(CAM_TOP_DOWN); break;
        case '4': switchCamera(CAM_DOLL_CLOSEUP); break; // 360° Doll Orbit & Arena Surroundings!
        case '5': switchCamera(CAM_SIDE_LEFT); break;
        case '6': switchCamera(CAM_SIDE_RIGHT); break;
        case '7': switchCamera(CAM_DOLL_POV); break;
        case '8': switchCamera(CAM_START_LINE); break;
        case '9': switchCamera(CAM_FINISH_LINE); break;
        case '0': switchCamera(CAM_OVERHEAD); break;
        case 'v': case 'V': case 'n': case 'N':
            CameraManager::cycleMode(camMode);
            break;

        case 'f': case 'F':
            Renderer::toggleShading();
            break;
        case 'r': case 'R':
            resetGame();
            break;
        case ' ':
            if (gameState == WAITING) {
                startGame();
            }
            break;
        case 27: // Esc
            AudioManager::cleanup();
            exit(0);
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    int modifiers = glutGetModifiers();
    keyHoldSprint = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
    keyHoldWalk   = (modifiers & (GLUT_ACTIVE_CTRL | GLUT_ACTIVE_ALT)) != 0;

    switch (key) {
        case 'w': case 'W': keyForward = false; break;
        case 's': case 'S': keyBackward = false; break;
        case 'a': case 'A': keyTurnLeft = false; break;
        case 'd': case 'D': keyTurnRight = false; break;
        case 'q': case 'Q': keyStrafeLeft = false; break;
        case 'e': case 'E': keyStrafeRight = false; break;
    }
}

void specialKeys(int key, int x, int y) {
    if (gameState == WAITING) {
        startGame();
        return;
    }
    if (key == GLUT_KEY_UP)    keyForward = true;
    if (key == GLUT_KEY_DOWN)  keyBackward = true;
    if (key == GLUT_KEY_LEFT)  keyTurnLeft = true;
    if (key == GLUT_KEY_RIGHT) keyTurnRight = true;
    
    int modifiers = glutGetModifiers();
    keyHoldSprint = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
    keyHoldWalk   = (modifiers & (GLUT_ACTIVE_CTRL | GLUT_ACTIVE_ALT)) != 0;
}

void specialKeysUp(int key, int x, int y) {
    if (key == GLUT_KEY_UP)    keyForward = false;
    if (key == GLUT_KEY_DOWN)  keyBackward = false;
    if (key == GLUT_KEY_LEFT)  keyTurnLeft = false;
    if (key == GLUT_KEY_RIGHT) keyTurnRight = false;

    int modifiers = glutGetModifiers();
    keyHoldSprint = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
    keyHoldWalk   = (modifiers & (GLUT_ACTIVE_CTRL | GLUT_ACTIVE_ALT)) != 0;
}

void timer(int value) {
    float dt = DT;

    if (gameState == PLAYING) {
        // Countdown timer tick
        gameTimer -= dt;
        if (gameTimer <= 0.0f) {
            gameTimer = 0.0f;
            checkPlayerElimination(false);
        }

        // Update Doll
        DollManager::update(doll, dt);

        // Update Guards
        GuardsManager::update(guards, dt);

        // Update NPCs
        ContestantsManager::update(player, npcs, guards, doll, particles, aliveCount, dt);

        // Player Controls & Multi-Speed Movement
        if (player.isAlive) {
            if (keyTurnLeft)  player.facingAngle += TURN_SPEED;
            if (keyTurnRight) player.facingAngle -= TURN_SPEED;

            float rad = player.facingAngle * 3.14159265f / 180.0f;
            float forwardX = sinf(rad);
            float forwardZ = cosf(rad);
            float rightX = cosf(rad);
            float rightZ = -sinf(rad);

            // Determine active speed based on keys and gear selection
            SpeedTier effectiveTier = player.speedTier;
            if (keyHoldSprint) {
                effectiveTier = SPEED_SPRINT;
            } else if (keyHoldWalk) {
                effectiveTier = SPEED_WALK;
            }

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
            } else {
                currentSpeed = SPEED_EASY_RUN;
                animFreq = WALK_OMEGA;
                animAmp  = WALK_AMPLITUDE;
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

            // Boundary constraints inside arena
            if (player.x < -ARENA_HALF_WIDTH + 0.8f) player.x = -ARENA_HALF_WIDTH + 0.8f;
            if (player.x >  ARENA_HALF_WIDTH - 0.8f) player.x =  ARENA_HALF_WIDTH - 0.8f;
            if (player.z < -ARENA_HALF_LENGTH + 0.8f) player.z = -ARENA_HALF_LENGTH + 0.8f;

            // Player Walk Cycle Animation
            if (isMoving) {
                player.walkTime += dt;
                player.legAngle = animAmp * sinf(animFreq * player.walkTime);
                player.armAngle = -player.legAngle;
            } else {
                player.legAngle = 0.0f;
                player.armAngle = 0.0f;
            }

            // Check if player moved during Red Light
            checkPlayerElimination(isMoving);
        } else {
            // Player death fall animation
            if (player.deathTimer < 1.0f) {
                player.deathTimer += dt * 3.0f;
                if (player.deathTimer > 1.0f) player.deathTimer = 1.0f;
            }
        }
    }

    // Update Particles
    ContestantsManager::updateParticles(particles, dt);

    // Update Camera (Passing active camera mode)
    CameraManager::update(player, doll, camMode, dt);

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// -----------------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------------
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Squid Game 3D: Red Light, Green Light");

    srand((unsigned int)time(NULL));

    Renderer::initGL();
    AudioManager::init();
    EnvironmentManager::init();
    UIManager::init();

    resetGame();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();

    AudioManager::cleanup();
    return 0;
}
