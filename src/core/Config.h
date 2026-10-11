#ifndef CONFIG_H
#define CONFIG_H

#include <glm/glm.hpp>

// =============================================================================
// SQUID GAME: MASTER CONFIGURATION & LAB SETTINGS
// Organized cleanly so any parameter can be modified instantly during evaluation.
// =============================================================================

// -----------------------------------------------------------------------------
// 1. GAME TIMERS & RULES
// -----------------------------------------------------------------------------
const float GAME_TIME_LIMIT     = 60.0f;   // RLGL countdown timer in seconds
const float GB_TIME_LIMIT       = 960.0f;  // Glass Bridge countdown (16 min = 960s)
const int   TOTAL_CONTESTANTS   = 456;     // Total contestants in Squid Game lore
const int   NUM_NPC_CONTESTANTS = 32;      // 3D NPC contestants rendered across line
const int   PLAYER_NUMBER        = 456;     // One shared player identity in every round

// -----------------------------------------------------------------------------
// 2. PLAYER & NPC SPEEDS (Movement & Animation)
// -----------------------------------------------------------------------------
const float SPEED_SLOW_WALK     = 0.038f;  // Precision slow walk (Z key)
const float SPEED_EASY_RUN      = 0.080f;  // Standard natural jog (X key / default)
const float SPEED_FAST_SPRINT   = 0.155f;  // Fast sprint (C key)
const float TURN_SPEED          = 2.8f;    // Player turning speed (deg/tick)
const float WALK_AMPLITUDE      = 28.0f;   // Leg & arm swing max angle (deg)
const float WALK_OMEGA          = 10.0f;   // Walk oscillation frequency
const float DT                  = 0.016f;  // Physics tick interval (~60 FPS)

// -----------------------------------------------------------------------------
// 3. COLOR PALETTES & MATERIALS (Change any color in 1 second!)
// -----------------------------------------------------------------------------
// (a) Contestants
const glm::vec3 COLOR_TRACKSUIT       = glm::vec3(0.10f, 0.50f, 0.44f); // Authentic Squid Game Teal Green
const glm::vec3 COLOR_TRACKSUIT_WHITE = glm::vec3(0.96f, 0.96f, 0.96f); // Clean white side stripes & shoes
const glm::vec3 COLOR_SKIN            = glm::vec3(0.92f, 0.74f, 0.60f); // Human skin tone
const glm::vec3 COLOR_HAIR            = glm::vec3(0.12f, 0.10f, 0.08f); // Dark hair
const glm::vec3 COLOR_SHOE_SOLE       = glm::vec3(0.20f, 0.20f, 0.22f);
const glm::vec3 COLOR_BIB_TEXT        = glm::vec3(0.03f, 0.06f, 0.06f);

// (b) Guards
const glm::vec3 COLOR_GUARD_MAGENTA   = glm::vec3(0.88f, 0.08f, 0.42f); // Authentic Squid Game Hot Magenta
const glm::vec3 COLOR_GUARD_BLACK     = glm::vec3(0.08f, 0.08f, 0.09f); // Tactical belts, harness, boots, mask
const glm::vec3 COLOR_GUARD_SILVER    = glm::vec3(0.85f, 0.85f, 0.88f); // Belt buckle

// (c) Doll (Young-hee)
const glm::vec3 COLOR_DOLL_ORANGE     = glm::vec3(0.92f, 0.38f, 0.08f); // Signature Orange Pinafore Dress
const glm::vec3 COLOR_DOLL_YELLOW     = glm::vec3(0.95f, 0.82f, 0.15f); // Signature Yellow Collar Shirt

// (d) Arena Environment (Red Light Green Light)
const glm::vec3 COLOR_ARENA_SAND_TINT = glm::vec3(1.00f, 0.97f, 0.93f); // Tint multiplied onto the sand texture
const glm::vec3 COLOR_ARENA_SKY_TOP   = glm::vec3(0.15f, 0.48f, 0.88f);
const glm::vec3 COLOR_ARENA_SKY       = glm::vec3(0.48f, 0.74f, 0.96f); // Lower sky gradient
const glm::vec3 COLOR_ARENA_CLOUD     = glm::vec3(0.96f, 0.97f, 1.00f);
const glm::vec3 COLOR_SKY_ZENITH      = glm::vec3(0.10f, 0.36f, 0.78f); // Sky dome, straight up
const glm::vec3 COLOR_SKY_HORIZON     = glm::vec3(0.66f, 0.84f, 0.98f); // Sky dome, at the horizon
const glm::vec3 COLOR_ARENA_WHEAT     = glm::vec3(0.78f, 0.59f, 0.20f);
const glm::vec3 COLOR_ARENA_HORIZON   = glm::vec3(0.30f, 0.56f, 0.22f);
const glm::vec3 COLOR_FINISH_LINE     = glm::vec3(0.95f, 0.12f, 0.48f); // Squid Game Pink finish line
const glm::vec3 COLOR_HOUSE_BODY      = glm::vec3(0.90f, 0.88f, 0.80f);
const glm::vec3 COLOR_HOUSE_ROOF      = glm::vec3(0.94f, 0.34f, 0.08f);

// (e) Glass Bridge
const glm::vec3 COLOR_GB_PINK_RAIL    = glm::vec3(0.95f, 0.12f, 0.48f); // Neon Pink perimeter & runner rails
const glm::vec3 COLOR_GB_CYAN         = glm::vec3(0.02f, 0.72f, 1.00f);
const glm::vec3 COLOR_GB_WALL         = glm::vec3(0.80f, 0.82f, 0.88f); // Tint on the concrete wall texture
const glm::vec3 COLOR_GB_CEILING      = glm::vec3(0.46f, 0.48f, 0.54f);
const glm::vec3 COLOR_GB_ABYSS        = glm::vec3(0.30f, 0.32f, 0.40f); // Pit floor far below
const glm::vec3 COLOR_GB_PLATFORM     = glm::vec3(0.78f, 0.80f, 0.86f);
const glm::vec3 COLOR_GB_STEEL_BEAM   = glm::vec3(0.24f, 0.28f, 0.36f); // Industrial I-beams & structural frames
const glm::vec3 COLOR_GB_STEEL_LIGHT  = glm::vec3(0.46f, 0.52f, 0.62f);
const glm::vec3 COLOR_GB_LAMP         = glm::vec3(1.00f, 0.93f, 0.82f); // Warm white wall lamps
const glm::vec3 COLOR_GB_GLASS        = glm::vec3(0.40f, 0.78f, 1.00f); // Tempered glass panels
const glm::vec3 COLOR_GB_SAFE         = glm::vec3(0.10f, 0.95f, 0.40f);

// (f) Scene lighting. These are the main artistic controls for each round.
const glm::vec3 COLOR_RLGL_CLEAR       = glm::vec3(0.24f, 0.55f, 0.88f);
const glm::vec3 COLOR_RLGL_SUN         = glm::vec3(1.00f, 0.88f, 0.72f);
const glm::vec3 COLOR_RLGL_POINT       = glm::vec3(1.00f, 0.72f, 0.42f);
const glm::vec3 COLOR_GB_CLEAR         = glm::vec3(0.30f, 0.34f, 0.42f);
const glm::vec3 COLOR_GB_FOG           = glm::vec3(0.36f, 0.40f, 0.50f);
const glm::vec3 COLOR_GB_FLOOD         = glm::vec3(0.62f, 0.74f, 1.00f);
const glm::vec3 COLOR_GB_POINT         = glm::vec3(1.00f, 0.70f, 0.38f);
const glm::vec3 COLOR_GB_SPOT          = glm::vec3(0.64f, 0.86f, 1.00f);

// -----------------------------------------------------------------------------
// 4. ARENA DIMENSIONS (Red Light Green Light)
// -----------------------------------------------------------------------------
const float ARENA_HALF_WIDTH    = 24.0f;   // Arena spans x = -24 .. +24
const float ARENA_HALF_LENGTH   = 75.0f;   // Arena spans z = -75 .. +75
const float START_Z             = -52.0f;  // Starting line
const float FINISH_Z            =  45.0f;  // Finish line (97 units distance)
const float WALL_HEIGHT         = 22.0f;   // Perimeter wall height

// -----------------------------------------------------------------------------
// 5. DOLL SPECIFICATIONS
// -----------------------------------------------------------------------------
const float DOLL_X              = 0.0f;
const float DOLL_Y              = 0.0f;
const float DOLL_Z              = 58.0f;
const float DOLL_SCALE          = 2.6f;    // Colossal Young-hee Doll scale

// -----------------------------------------------------------------------------
// 6. GLASS BRIDGE GEOMETRY & RULES
// -----------------------------------------------------------------------------
const int   GB_NUM_STEPS         = 16;     // 16 pairs of glass panels
const int   GB_TOTAL_PLAYERS     = 16;     // 16 contestants in queue
const float GB_BRIDGE_START_Z    = 3.0f;   // Z where glass panels start
const float GB_STEP_LENGTH       = 3.3f;   // Center-to-center distance per step
const float GB_BRIDGE_HEIGHT     = 16.0f;  // Elevation high above bottomless void
const float GB_LEFT_PANEL_X      =  1.35f; // Center X of left glass pane
const float GB_RIGHT_PANEL_X     = -1.35f; // Center X of right glass pane
const float GB_PANEL_WIDTH       =  1.90f; // Width of glass pane
const float GB_PANEL_LENGTH      =  2.40f; // Length of glass pane
const float GB_PANEL_THICKNESS   =  0.10f; // Thickness of glass pane

// -----------------------------------------------------------------------------
// 7. CAMERA ORBIT & BOUNDS
// -----------------------------------------------------------------------------
const float CAM_DISTANCE_3RD    = 5.2f;
const float CAM_HEIGHT_3RD      = 2.2f;
const float CAM_DISTANCE_DOLL   = 7.5f;
const float CAM_MIN_DIST        = 1.5f;
const float CAM_MAX_DIST        = 45.0f;

// -----------------------------------------------------------------------------
// 8. HUD & UI SIDEBARS
// -----------------------------------------------------------------------------
const int UI_LEFT_BAR_WIDTH     = 255;
const int UI_RIGHT_BAR_WIDTH    = 280;

#endif // CONFIG_H
