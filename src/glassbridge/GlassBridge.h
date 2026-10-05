#ifndef GLASSBRIDGE_H
#define GLASSBRIDGE_H

#include "core/Config.h"
#include "core/Types.h"
#include <vector>

class GlassBridgeManager {
public:
    // Initialize or reset a Glass Bridge game session
    static void init(std::vector<GlassStep>& steps, 
                     std::vector<GBContestant>& contestants, 
                     std::vector<GlassShard>& shards,
                     int& playerQueueNum,
                     int& activeQueueIndex,
                     GlassBridgeState& state,
                     float& timer,
                     int forcePlayerQueueNum = -1);

    // Update game logic, AI choices, jump physics, falls, and particle shards
    static void update(std::vector<GlassStep>& steps,
                       std::vector<GBContestant>& contestants,
                       std::vector<GlassShard>& shards,
                       int& activeQueueIndex,
                       GlassBridgeState& state,
                       float dt,
                       bool fastForwardAI = false);

    // Player 456 manual jump action (A/Left = GLASS_LEFT, D/Right = GLASS_RIGHT)
    static bool jumpPlayer(std::vector<GlassStep>& steps,
                           std::vector<GBContestant>& contestants,
                           std::vector<GlassShard>& shards,
                           GlassSide chosenSide,
                           int activeQueueIndex,
                           GlassBridgeState& state);

    // Skip all remaining AI turns directly to Player 456's turn with instant simulated progression
    static void skipToPlayerTurn(std::vector<GlassStep>& steps,
                                 std::vector<GBContestant>& contestants,
                                 int& activeQueueIndex,
                                 GlassBridgeState& state);

    // 3D Scene Rendering
    static void drawEnvironment(float animTimer);
    static void drawGuard(float x, float y, float z, float angleY, char maskShape);
    static void drawBridge(const std::vector<GlassStep>& steps, float animTimer, const std::vector<GBContestant>& contestants, int activeQueueIndex);
    static void drawContestants(const std::vector<GBContestant>& contestants, int activeQueueIndex);
    static void drawShards(const std::vector<GlassShard>& shards);

    // Particle FX
    static void spawnShards(std::vector<GlassShard>& shards, float centerX, float centerY, float centerZ);

    // Camera view calculators for all 7 modes
    static void getCameraVectors(const std::vector<GBContestant>& contestants,
                                 int activeQueueIndex,
                                 GlassBridgeCamMode mode,
                                 float orbitYaw, float orbitPitch, float orbitDist,
                                 float& eyeX, float& eyeY, float& eyeZ,
                                 float& lookX, float& lookY, float& lookZ,
                                 float& upX, float& upY, float& upZ, float& fov);
};

#endif // GLASSBRIDGE_H
