#ifndef CONTESTANTS_H
#define CONTESTANTS_H

#include "core/Types.h"
#include <vector>

class ContestantsManager {
public:
    static void init(Player& player, std::vector<NPCContestant>& npcs);
    static void update(Player& player, std::vector<NPCContestant>& npcs, 
                       std::vector<Guard>& guards, const Doll& doll, 
                       std::vector<Particle>& particles,
                       int& aliveCount, float dt);
    static void drawPlayer(const Player& player);
    static void drawNPCs(const std::vector<NPCContestant>& npcs);
    // One canonical contestant mesh used by both games. Round-specific code supplies
    // only position and animation state; appearance and proportions stay identical.
    static void drawCharacter(float x, float y, float z, float facingAngle,
                              float legAngle, float armAngle, int number,
                              bool isAlive, float fallAngle, float scale = 1.15f,
                              bool drawDigits = true);
    static void drawParticles(const std::vector<Particle>& particles);
    static void spawnBloodParticles(std::vector<Particle>& particles, float x, float y, float z);
    static void updateParticles(std::vector<Particle>& particles, float dt);
};

#endif // CONTESTANTS_H
