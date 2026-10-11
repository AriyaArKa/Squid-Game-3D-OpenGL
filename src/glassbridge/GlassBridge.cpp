#include "glassbridge/GlassBridge.h"
#include "audio/Audio.h"
#include <cmath>
#include <cstdlib>

namespace {
    const float PI = 3.14159265358979323846f;
}

// -----------------------------------------------------------------------------------
// 1. INITIALIZATION & SESSION RESET
// -----------------------------------------------------------------------------------
void GlassBridgeManager::init(std::vector<GlassStep>& steps, 
                             std::vector<GBContestant>& contestants, 
                             std::vector<GlassShard>& shards,
                             int& playerQueueNum,
                             int& activeQueueIndex,
                             GlassBridgeState& state,
                             float& timer,
                             int forcePlayerQueueNum) {
    // Initialize 16 randomized glass pairs (50/50 safe probability)
    steps.resize(GB_NUM_STEPS);
    for (int i = 0; i < GB_NUM_STEPS; ++i) {
        steps[i].safeSide = (rand() % 2 == 0) ? GLASS_LEFT : GLASS_RIGHT;
        steps[i].leftShattered = false;
        steps[i].rightShattered = false;
        steps[i].leftRevealed = false;
        steps[i].rightRevealed = false;
    }

    if (forcePlayerQueueNum > 0 && forcePlayerQueueNum <= GB_TOTAL_PLAYERS) {
        playerQueueNum = forcePlayerQueueNum;
    } else {
        playerQueueNum = 1 + rand() % GB_TOTAL_PLAYERS;
    }

    activeQueueIndex = 0;
    state = GB_PLAYING;
    timer = GB_TIME_LIMIT;

    shards.clear();

    const int famousNumbers[16] = {
        67, 218, 1, 101, 199, 212, 240, 69,
        70, 96, 360, 21, 151, 17, 322, 456
    };

    // Initialize 16 contestants lined up on starting platform
    contestants.resize(GB_TOTAL_PLAYERS);
    for (int i = 0; i < GB_TOTAL_PLAYERS; ++i) {
        int qNum = i + 1;
        bool isP456 = (qNum == playerQueueNum);

        contestants[i].queueNumber = qNum;
        contestants[i].bibNumber = isP456 ? PLAYER_NUMBER : famousNumbers[i % 16];
        contestants[i].isPlayer456 = isP456;
        contestants[i].isAlive = true;
        contestants[i].hasFinished = false;
        contestants[i].currentStep = -1;
        contestants[i].currentSide = GLASS_NONE;

        float queueZ = (i == 0) ? 0.8f : (-0.8f - (float)i * 0.95f);
        float queueX = (i == 0) ? 0.0f : ((i % 2 == 0) ? -0.45f : 0.45f);

        contestants[i].x = queueX;
        contestants[i].y = GB_BRIDGE_HEIGHT;
        contestants[i].z = queueZ;

        contestants[i].targetX = queueX;
        contestants[i].targetY = GB_BRIDGE_HEIGHT;
        contestants[i].targetZ = queueZ;

        contestants[i].jumpStartX = queueX;
        contestants[i].jumpStartY = GB_BRIDGE_HEIGHT;
        contestants[i].jumpStartZ = queueZ;

        contestants[i].jumpProgress = 0.0f;
        contestants[i].isJumping = false;
        contestants[i].fallVelocity = 0.0f;
        contestants[i].legAngle = 0.0f;
        contestants[i].armAngle = 0.0f;
        contestants[i].aiThinkTimer = 1.0f + ((float)rand() / RAND_MAX) * 1.2f;
    }
}

// -----------------------------------------------------------------------------------
// 2. SKIP DIRECTLY TO PLAYER 456'S TURN (INSTANT SIMULATION)
// -----------------------------------------------------------------------------------
void GlassBridgeManager::skipToPlayerTurn(std::vector<GlassStep>& steps,
                                         std::vector<GBContestant>& contestants,
                                         int& activeQueueIndex,
                                         GlassBridgeState& state) {
    if (state != GB_PLAYING) return;

    int pIdx = -1;
    for (size_t i = 0; i < contestants.size(); ++i) {
        if (contestants[i].isPlayer456) {
            pIdx = (int)i;
            break;
        }
    }
    if (pIdx < 0 || activeQueueIndex >= pIdx) return;

    // Simulate contestants ahead of Player 456
    while (activeQueueIndex < pIdx) {
        GBContestant& npc = contestants[activeQueueIndex];
        if (!npc.isAlive || npc.hasFinished) {
            activeQueueIndex++;
            continue;
        }

        // Find current frontier
        int frontier = 0;
        while (frontier < GB_NUM_STEPS && (steps[frontier].leftRevealed || steps[frontier].rightRevealed)) {
            frontier++;
        }

        if (frontier >= GB_NUM_STEPS) {
            npc.currentStep = GB_NUM_STEPS;
            npc.hasFinished = true;
            activeQueueIndex++;
            continue;
        }

        // NPC guesses frontier step 50/50
        GlassStep& st = steps[frontier];
        GlassSide pick = (rand() % 2 == 0) ? GLASS_LEFT : GLASS_RIGHT;
        if (pick == st.safeSide) {
            // Survived step
            if (pick == GLASS_LEFT) st.leftRevealed = true;
            else st.rightRevealed = true;
            npc.currentStep = frontier;
        } else {
            // Shattered and eliminated
            if (pick == GLASS_LEFT) {
                st.leftShattered = true;
                st.rightRevealed = true;
            } else {
                st.rightShattered = true;
                st.leftRevealed = true;
            }
            npc.isAlive = false;
            npc.fallVelocity = 10.0f;
            npc.y = -35.0f; // placed in void
            activeQueueIndex++;
        }
    }

    // Now active is Player 456: advance them across known safe steps to the frontier!
    activeQueueIndex = pIdx;
    GBContestant& player = contestants[pIdx];
    int solvedStep = -1;
    while (solvedStep + 1 < GB_NUM_STEPS && (steps[solvedStep + 1].leftRevealed || steps[solvedStep + 1].rightRevealed)) {
        solvedStep++;
    }

    if (solvedStep >= 0 && solvedStep < GB_NUM_STEPS) {
        player.currentStep = solvedStep;
        GlassSide safeSide = steps[solvedStep].leftRevealed ? GLASS_LEFT : GLASS_RIGHT;
        player.currentSide = safeSide;
        player.x = (safeSide == GLASS_LEFT) ? GB_LEFT_PANEL_X : GB_RIGHT_PANEL_X;
        player.y = GB_BRIDGE_HEIGHT;
        player.z = GB_BRIDGE_START_Z + (float)solvedStep * GB_STEP_LENGTH;
        player.targetX = player.x;
        player.targetY = player.y;
        player.targetZ = player.z;
    } else {
        player.currentStep = -1;
        player.currentSide = GLASS_NONE;
        player.x = 0.0f;
        player.y = GB_BRIDGE_HEIGHT;
        player.z = 0.8f;
        player.targetX = 0.0f;
        player.targetY = GB_BRIDGE_HEIGHT;
        player.targetZ = 0.8f;
    }
}

// -----------------------------------------------------------------------------------
// 3. FRAME UPDATE & PHYSICS LOOP
// -----------------------------------------------------------------------------------
void GlassBridgeManager::update(std::vector<GlassStep>& steps,
                               std::vector<GBContestant>& contestants,
                               std::vector<GlassShard>& shards,
                               int& activeQueueIndex,
                               GlassBridgeState& state,
                               float dt,
                               bool fastForwardAI) {
    // Update Shards
    for (size_t i = 0; i < shards.size();) {
        shards[i].vy -= 28.0f * dt;
        shards[i].x += shards[i].vx * dt;
        shards[i].y += shards[i].vy * dt;
        shards[i].z += shards[i].vz * dt;

        shards[i].rx += shards[i].vrx * dt;
        shards[i].ry += shards[i].vry * dt;
        shards[i].rz += shards[i].vrz * dt;

        shards[i].life -= dt;
        shards[i].a = shards[i].life / 2.0f;
        if (shards[i].a < 0.0f) shards[i].a = 0.0f;

        if (shards[i].life <= 0.0f || shards[i].y < -40.0f) {
            shards.erase(shards.begin() + i);
        } else {
            ++i;
        }
    }

    // Update falling contestants
    for (size_t i = 0; i < contestants.size(); ++i) {
        if (!contestants[i].isAlive && contestants[i].fallVelocity >= 0.0f) {
            contestants[i].fallVelocity += 32.0f * dt;
            contestants[i].y -= contestants[i].fallVelocity * dt;
        }
    }

    if (state != GB_PLAYING) return;

    if (activeQueueIndex >= (int)contestants.size()) {
        bool pSurvived = false;
        for (const auto& c : contestants) {
            if (c.isPlayer456 && c.hasFinished) pSurvived = true;
        }
        state = pSurvived ? GB_WON : GB_ELIMINATED;
        return;
    }

    GBContestant& active = contestants[activeQueueIndex];

    // If active contestant is jumping
    if (active.isJumping) {
        float jumpSpeed = fastForwardAI ? 4.8f : 2.4f;
        active.jumpProgress += dt * jumpSpeed;

        if (active.jumpProgress < 1.0f) {
            float p = active.jumpProgress;
            active.x = active.jumpStartX + (active.targetX - active.jumpStartX) * p;
            active.z = active.jumpStartZ + (active.targetZ - active.jumpStartZ) * p;

            // Parabolic vertical jump arc
            float arcHeight = sinf(p * PI) * 1.30f;
            active.y = GB_BRIDGE_HEIGHT + arcHeight;

            active.legAngle = sinf(p * PI * 2.0f) * 25.0f;
            active.armAngle = -active.legAngle;
        } else {
            // Landed!
            active.isJumping = false;
            active.jumpProgress = 0.0f;
            active.x = active.targetX;
            active.y = GB_BRIDGE_HEIGHT;
            active.z = active.targetZ;
            active.legAngle = 0.0f;
            active.armAngle = 0.0f;

            int targetStep = active.currentStep + 1;

            if (targetStep < GB_NUM_STEPS) {
                GlassStep& step = steps[targetStep];
                GlassSide landedSide = active.currentSide;

                if (landedSide == step.safeSide) {
                    // SAFE! Tempered glass holds!
                    AudioManager::playGlassStep();
                    active.currentStep = targetStep;
                    if (landedSide == GLASS_LEFT) step.leftRevealed = true;
                    else step.rightRevealed = true;

                    active.aiThinkTimer = 0.8f + ((float)rand() / RAND_MAX) * 1.2f;
                } else {
                    // FRAGILE! Shatter & eliminate
                    AudioManager::playGlassShatter();
                    AudioManager::playFallWhoosh();

                    if (landedSide == GLASS_LEFT) {
                        step.leftShattered = true;
                        step.rightRevealed = true;
                    } else {
                        step.rightShattered = true;
                        step.leftRevealed = true;
                    }

                    spawnShards(shards, active.x, active.y, active.z);

                    active.isAlive = false;
                    active.fallVelocity = 0.1f;

                    if (active.isPlayer456) {
                        state = GB_ELIMINATED;
                        AudioManager::playGameOverMusic();
                    } else {
                        activeQueueIndex++;
                    }
                }
            } else {
                // Reached Destination Finish Platform!
                active.currentStep = GB_NUM_STEPS;
                active.hasFinished = true;
                AudioManager::playGlassStep();

                if (active.isPlayer456) {
                    state = GB_WON;
                    AudioManager::playWinMusic();
                }

                activeQueueIndex++;
            }
        }
        return;
    }

    // If active contestant is standing and waiting to jump
    if (active.isAlive && !active.isJumping) {
        // If standing behind on platform, step up to front bridge edge (Z = 0.8)
        if (active.currentStep == -1 && active.z < 0.8f) {
            active.z += dt * 4.0f;
            if (active.z > 0.8f) active.z = 0.8f;
            active.x += (0.0f - active.x) * dt * 5.0f;
            active.legAngle = sinf(active.z * 12.0f) * 22.0f;
            active.armAngle = -active.legAngle;
            return;
        }

        // 1. Advance across known revealed safe steps (applies to both NPC and Player 456!)
        int nextStep = active.currentStep + 1;
        if (nextStep < GB_NUM_STEPS) {
            GlassStep& st = steps[nextStep];
            if (st.leftRevealed || st.rightRevealed) {
                GlassSide safeChoice = st.leftRevealed ? GLASS_LEFT : GLASS_RIGHT;
                active.jumpStartX = active.x;
                active.jumpStartY = active.y;
                active.jumpStartZ = active.z;

                active.targetX = (safeChoice == GLASS_LEFT) ? GB_LEFT_PANEL_X : GB_RIGHT_PANEL_X;
                active.targetY = GB_BRIDGE_HEIGHT;
                active.targetZ = GB_BRIDGE_START_Z + (float)nextStep * GB_STEP_LENGTH;
                active.currentSide = safeChoice;

                active.isJumping = true;
                active.jumpProgress = 0.0f;
                return;
            }
        }

        // 2. If at step 15 (last step), jump to Finish Platform!
        if (active.currentStep == GB_NUM_STEPS - 1) {
            if (!active.isPlayer456) {
                active.jumpStartX = active.x;
                active.jumpStartY = active.y;
                active.jumpStartZ = active.z;

                active.targetX = 0.0f;
                active.targetY = GB_BRIDGE_HEIGHT;
                active.targetZ = GB_BRIDGE_START_Z + (float)GB_NUM_STEPS * GB_STEP_LENGTH + 2.5f;
                active.currentSide = GLASS_NONE;

                active.isJumping = true;
                active.jumpProgress = 0.0f;
                return;
            }
        }

        // 3. At an unknown frontier step:
        if (!active.isPlayer456) {
            float speedMod = fastForwardAI ? 6.0f : 1.0f;
            active.aiThinkTimer -= dt * speedMod;
            if (active.aiThinkTimer <= 0.0f) {
                GlassSide pick = (rand() % 2 == 0) ? GLASS_LEFT : GLASS_RIGHT;

                active.jumpStartX = active.x;
                active.jumpStartY = active.y;
                active.jumpStartZ = active.z;

                active.targetX = (pick == GLASS_LEFT) ? GB_LEFT_PANEL_X : GB_RIGHT_PANEL_X;
                active.targetY = GB_BRIDGE_HEIGHT;
                active.targetZ = GB_BRIDGE_START_Z + (float)nextStep * GB_STEP_LENGTH;
                active.currentSide = pick;

                active.isJumping = true;
                active.jumpProgress = 0.0f;
            }
        } else {
            // Player 456 standing at frontier step ready for user input
            active.legAngle = 0.0f;
            active.armAngle = 0.0f;
        }
    }
}

// -----------------------------------------------------------------------------------
// 4. MANUAL PLAYER JUMP ACTION
// -----------------------------------------------------------------------------------
bool GlassBridgeManager::jumpPlayer(std::vector<GlassStep>& steps,
                                   std::vector<GBContestant>& contestants,
                                   std::vector<GlassShard>& shards,
                                   GlassSide chosenSide,
                                   int activeQueueIndex,
                                   GlassBridgeState& state) {
    if (state != GB_PLAYING) return false;
    if (activeQueueIndex >= (int)contestants.size()) return false;

    GBContestant& player = contestants[activeQueueIndex];
    if (!player.isPlayer456 || !player.isAlive || player.isJumping) return false;

    int targetStep = player.currentStep + 1;

    player.jumpStartX = player.x;
    player.jumpStartY = player.y;
    player.jumpStartZ = player.z;

    if (targetStep < GB_NUM_STEPS) {
        player.targetX = (chosenSide == GLASS_LEFT) ? GB_LEFT_PANEL_X : GB_RIGHT_PANEL_X;
        player.targetY = GB_BRIDGE_HEIGHT;
        player.targetZ = GB_BRIDGE_START_Z + (float)targetStep * GB_STEP_LENGTH;
        player.currentSide = chosenSide;
    } else {
        // Final jump to Destination Platform!
        player.targetX = 0.0f;
        player.targetY = GB_BRIDGE_HEIGHT;
        player.targetZ = GB_BRIDGE_START_Z + (float)GB_NUM_STEPS * GB_STEP_LENGTH + 2.5f;
        player.currentSide = GLASS_NONE;
    }

    player.isJumping = true;
    player.jumpProgress = 0.0f;
    return true;
}
