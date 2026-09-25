#ifndef TYPES_H
#define TYPES_H

enum GameState {
    WAITING,
    PLAYING,
    WON,
    CAUGHT,
    TIME_UP
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
    CAM_FIRST_PERSON = 2, // 2: 1st person eyes
    CAM_TOP_DOWN     = 3, // 3: Aerial Top Floor View of entire arena
    CAM_DOLL_CLOSEUP = 4, // 4: Doll 360° Orbit & Arena Surroundings
    CAM_SIDE_LEFT    = 5, // 5: Left stadium broadcast
    CAM_SIDE_RIGHT   = 6, // 6: Right stadium broadcast
    CAM_DOLL_POV     = 7, // 7: Looking from Doll down at players
    CAM_START_LINE   = 8, // 8: Ground starting line cam
    CAM_FINISH_LINE  = 9, // 9: Finish line gate cam
    CAM_OVERHEAD     = 0  // 0: High panoramic stadium drone (360° Orbit)
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
    float reactionDelay;
    bool willMakeMistake;
    float mistakeTriggerTime; // Random time during Red Light when they flinch/move
    bool hasMovedDuringRed;
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
