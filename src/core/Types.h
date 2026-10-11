#ifndef TYPES_H
#define TYPES_H

#include <glm/glm.hpp>

// -----------------------------------------------------------------------------
// 3-TYPE LIGHTING & SHADING LAB CONFIGURATION
// -----------------------------------------------------------------------------
struct DirLightData {
    bool enabled = true;
    glm::vec3 direction = glm::vec3(-0.4f, -1.0f, -0.3f);
    glm::vec3 color = glm::vec3(0.78f, 0.74f, 0.70f);
    float ambientIntensity = 0.35f;
    float diffuseIntensity = 0.65f;
    float specularIntensity = 0.30f;
};

struct PointLightData {
    bool enabled = true;
    glm::vec3 position = glm::vec3(0.0f, 18.0f, 25.0f);
    glm::vec3 color = glm::vec3(1.0f, 0.88f, 0.62f);
    float constant = 1.0f;
    float linear = 0.035f;
    float quadratic = 0.005f;
    float intensity = 1.2f;
};

struct SpotLightData {
    bool enabled = true;
    glm::vec3 position = glm::vec3(0.0f, 6.5f, 58.0f);
    glm::vec3 direction = glm::vec3(0.0f, -0.15f, -1.0f);
    glm::vec3 color = glm::vec3(1.0f, 0.20f, 0.20f);
    float cutOff = 0.965f;      // cos(15 degrees)
    float outerCutOff = 0.920f; // cos(23 degrees)
    float intensity = 2.0f;
};

// Extra point lamps (e.g. the wall lamps along both sides of the glass bridge hall).
// They are ordinary point lights and switch on/off together with the main point light.
const int MAX_FILL_LIGHTS = 4;

struct SceneLighting {
    DirLightData dir;
    PointLightData point;                  // Main point light (key 8)
    PointLightData fill[MAX_FILL_LIGHTS];  // Extra point lamps, same equation as `point`
    int numFill = 0;
    SpotLightData spot;

    // Hemisphere ambient: surfaces facing up get `ambientSky`, facing down get `ambientGround`.
    glm::vec3 ambientSky = glm::vec3(0.46f, 0.58f, 0.82f);
    glm::vec3 ambientGround = glm::vec3(0.42f, 0.29f, 0.16f);

    // Distance haze: far surfaces fade toward fogColor (adds depth to large spaces).
    bool fogEnabled = false;
    glm::vec3 fogColor = glm::vec3(0.6f, 0.7f, 0.85f);
    float fogDensity = 0.0f;
};

enum GamePhase {
    PHASE_RED_LIGHT_GREEN_LIGHT = 0,
    PHASE_GLASS_BRIDGE          = 1
};

enum GameState {
    WAITING,
    PLAYING,
    WON,
    CAUGHT,
    TIME_UP
};

enum GlassBridgeState {
    GB_WAITING,
    GB_PLAYING,
    GB_WON,
    GB_ELIMINATED
};

enum GlassBridgeCamMode {
    GB_CAM_FOLLOW        = 1, // Follow active jumper / Player 456
    GB_CAM_RUNWAY        = 2, // Runway view down the bridge (matches Image 1)
    GB_CAM_OVER_SHOULDER = 3, // Over the shoulder of waiting queue (matches Image 2)
    GB_CAM_ORBIT_360     = 4, // Free 360° mouse drag orbit & zoom
    GB_CAM_TOP_DOWN      = 5, // Top aerial view
    GB_CAM_SIDE_PROFILE  = 6, // Side broadcast view
    GB_CAM_DESTINATION   = 7  // Looking back from finish platform
};

enum GlassSide {
    GLASS_NONE  = -1,
    GLASS_LEFT  = 0,
    GLASS_RIGHT = 1
};

struct GlassStep {
    GlassSide safeSide;        // GLASS_LEFT or GLASS_RIGHT
    bool leftShattered;
    bool rightShattered;
    bool leftRevealed;
    bool rightRevealed;
};

struct GlassShard {
    float x, y, z;
    float vx, vy, vz;
    float rx, ry, rz;
    float vrx, vry, vrz;
    float size;
    float r, g, b, a;
    float life;
};

struct GBContestant {
    int queueNumber;         // 1 to 16
    int bibNumber;           // e.g. 456, 067, 218...
    bool isPlayer456;
    bool isAlive;
    bool hasFinished;
    int currentStep;         // -1 = on start platform, 0..15 = on bridge step, 16 = on finish platform
    GlassSide currentSide;   // GLASS_LEFT or GLASS_RIGHT
    float x, y, z;
    float targetX, targetY, targetZ;
    float jumpStartX, jumpStartY, jumpStartZ;
    float jumpProgress;      // 0.0 to 1.0 during jump arc
    bool isJumping;
    float fallVelocity;
    float legAngle, armAngle;
    float aiThinkTimer;
};

enum LightState {
    GREEN_LIGHT,
    RED_LIGHT
};

enum GuardType {
    GUARD_CIRCLE,    // Worker
    GUARD_TRIANGLE,  // Soldier
    GUARD_SQUARE     // Manager
};

enum SpeedTier {
    SPEED_WALK,      // Precision slow walk
    SPEED_JOG,       // Standard easy run / normal jog
    SPEED_SPRINT     // Fast sprint
};

enum CameraMode {
    CAM_THIRD_PERSON = 1, // 1: Behind Player 456 (360° Orbit)
    CAM_OVERHEAD     = 2, // 2: Arena Free 360° Cam (Zoom in/out to any point!)
    CAM_TOP_DOWN     = 3, // 3: Aerial Top View of entire arena
    CAM_DOLL_CLOSEUP = 4, // 4: Doll 360° Orbit & Arena Surroundings
    CAM_SIDE_LEFT    = 5, // 5: Left stadium broadcast
    CAM_SIDE_RIGHT   = 6, // 6: Right stadium broadcast
    CAM_FINISH_LINE  = 7  // 7: Finish line 360° Cam (looking down at incoming players)
};

struct Player {
    float x, y, z;
    float facingAngle;   // degrees around Y
    float legAngle;
    float armAngle;
    float walkTime;
    bool isAlive;
    float deathTimer;
    int playerNumber;
    SpeedTier speedTier;
    float currentSpeed;
};

struct Doll {
    float x, y, z;
    float headAngle;     // 0 = facing tree/away (Green), 180 = facing players (Red)
    float targetAngle;
    LightState lightState;
    float stateTimer;
    float chantProgress;
    float chantDuration;
    bool isTurning;
    float scanAlpha;
};

struct Guard {
    float x, y, z;
    float facingAngle;
    GuardType type;
    bool isShooting;
    float shootTimer;
};

struct NPCContestant {
    float x, y, z;
    float facingAngle;
    float legAngle;
    float armAngle;
    float walkTime;
    float speed;
    int number;
    bool isAlive;
    float fallAngle;
    bool hasMovedDuringRed; // Set during Red Light if NPC moved/eliminated
};

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float r, g, b, a;
    float size;
    float life;
    float maxLife;
};

#endif // TYPES_H
