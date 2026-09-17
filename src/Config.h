#ifndef CONFIG_H
#define CONFIG_H

// Grand Arena layout dimensions (Long realistic path that takes real time to cross)
const float ARENA_HALF_WIDTH  = 24.0f;   // arena spans x = -24 .. +24
const float ARENA_HALF_LENGTH = 75.0f;   // arena spans z = -75 .. +75 (grand stadium scale)
const float START_Z           = -52.0f;  // contestants spawn line at -52
const float FINISH_Z          =  45.0f;  // finish line at +45 (97 units running distance!)
const float WALL_HEIGHT       = 22.0f;   // tall sky walls

// Player movement & multi-speed control (Walk, Easy Run/Jog, Fast Sprint)
const float SPEED_SLOW_WALK   = 0.038f;     // Slow precision walk (safe, stops instantly)
const float SPEED_EASY_RUN    = 0.080f;     // Standard natural jog / easy run
const float SPEED_FAST_SPRINT = 0.155f;     // Fast full sprint (high speed, riskier)
const float TURN_SPEED        = 2.8f;       // degrees turned per tick
const float WALK_AMPLITUDE    = 28.0f;      // leg swing max angle
const float WALK_OMEGA        = 10.0f;      // walk cycle frequency
const float DT                = 0.016f;     // timer tick interval (~60 FPS)

// Camera tuning & 360 orbit bounds
const float CAM_DISTANCE_3RD  = 5.2f;
const float CAM_HEIGHT_3RD    = 2.2f;
const float CAM_DISTANCE_DOLL = 7.5f;
const float CAM_MIN_DIST      = 1.5f;
const float CAM_MAX_DIST      = 45.0f;

// Game timer (5 minutes = 05:00)
const float GAME_TIME_LIMIT = 300.0f;

// Doll position & scale
const float DOLL_X = 0.0f;
const float DOLL_Y = 0.0f;
const float DOLL_Z = 58.0f;
const float DOLL_SCALE = 2.6f;           // Colossal Young-hee Doll

// Total contestants in the game
const int TOTAL_CONTESTANTS = 456;
const int NUM_NPC_CONTESTANTS = 32;      // 32 detailed 3D contestants across the line

#endif // CONFIG_H
