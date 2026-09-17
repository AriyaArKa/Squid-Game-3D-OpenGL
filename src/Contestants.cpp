#include "Contestants.h"
#include "Guards.h"
#include "Renderer.h"
#include "Audio.h"
#include "Config.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>

namespace {
    // Render 3D numeric digits on player patches
    void drawDigit3D(int d, float size) {
        static const bool segs[10][7] = {
            {1, 1, 1, 1, 1, 1, 0}, // 0
            {0, 1, 1, 0, 0, 0, 0}, // 1
            {1, 1, 0, 1, 1, 0, 1}, // 2
            {1, 1, 1, 1, 0, 0, 1}, // 3
            {0, 1, 1, 0, 0, 1, 1}, // 4
            {1, 0, 1, 1, 0, 1, 1}, // 5
            {1, 0, 1, 1, 1, 1, 1}, // 6
            {1, 1, 1, 0, 0, 0, 0}, // 7
            {1, 1, 1, 1, 1, 1, 1}, // 8
            {1, 1, 1, 1, 0, 1, 1}  // 9
        };
        if (d < 0 || d > 9) return;
        float w = size * 0.45f;
        float h = size * 0.48f;
        float t = size * 0.09f;

        Renderer::setMaterial(0.06f, 0.18f, 0.16f, 10.0f); // Dark teal printed numeral
        if (segs[d][0]) { glPushMatrix(); glTranslatef(0.0f, h, 0.0f); Renderer::drawBox(w, t, 0.015f); glPopMatrix(); }
        if (segs[d][1]) { glPushMatrix(); glTranslatef(w*0.5f, h*0.5f, 0.0f); Renderer::drawBox(t, h, 0.015f); glPopMatrix(); }
        if (segs[d][2]) { glPushMatrix(); glTranslatef(w*0.5f, -h*0.5f, 0.0f); Renderer::drawBox(t, h, 0.015f); glPopMatrix(); }
        if (segs[d][3]) { glPushMatrix(); glTranslatef(0.0f, -h, 0.0f); Renderer::drawBox(w, t, 0.015f); glPopMatrix(); }
        if (segs[d][4]) { glPushMatrix(); glTranslatef(-w*0.5f, -h*0.5f, 0.0f); Renderer::drawBox(t, h, 0.015f); glPopMatrix(); }
        if (segs[d][5]) { glPushMatrix(); glTranslatef(-w*0.5f, h*0.5f, 0.0f); Renderer::drawBox(t, h, 0.015f); glPopMatrix(); }
        if (segs[d][6]) { glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f); Renderer::drawBox(w, t, 0.015f); glPopMatrix(); }
    }

    void draw3DigitNumber(int num, float size) {
        int d1 = (num / 100) % 10;
        int d2 = (num / 10) % 10;
        int d3 = num % 10;
        float spacing = size * 0.58f;
        glPushMatrix(); glTranslatef(-spacing, 0.0f, 0.0f); drawDigit3D(d1, size); glPopMatrix();
        glPushMatrix(); glTranslatef(0.0f,     0.0f, 0.0f); drawDigit3D(d2, size); glPopMatrix();
        glPushMatrix(); glTranslatef(spacing,  0.0f, 0.0f); drawDigit3D(d3, size); glPopMatrix();
    }

    // Classic proportional human contestant figure with custom numbers
    void drawContestantFigure(float legAngle, float armAngle, int number, bool isAlive, float fallAngle) {
        // Tracksuit Colors: Authentic Squid Game Teal Green
        const float tr = 0.10f, tg = 0.50f, tb = 0.44f;
        const float wr = 0.96f, wg = 0.96f, wb = 0.96f;

        glPushMatrix();

        if (!isAlive) {
            // Fallen contestant
            glRotatef(fallAngle, 1.0f, 0.0f, 0.2f);
            glTranslatef(0.0f, -fallAngle * 0.008f, 0.0f);
        }

        // Ground shadow
        if (isAlive) {
            Renderer::drawGroundShadow(0.0f, 0.0f, 0.40f, 0.36f, 0.35f);
        }

        // 1. LEGS (Classic Proportional Tracksuit Legs + White Slip-on Shoes)
        const float legLen = 0.62f;
        const float hipY = legLen;

        for (int side = -1; side <= 1; side += 2) {
            float sx = side * 0.15f;
            float lAngle = (side == -1) ? legAngle : -legAngle;

            glPushMatrix();
            glTranslatef(sx, hipY, 0.0f); // Hip joint pivot
            glRotatef(lAngle, 1.0f, 0.0f, 0.0f);

            // Green Trouser leg
            Renderer::setMaterial(tr, tg, tb, 20.0f);
            glPushMatrix();
            glTranslatef(0.0f, -legLen * 0.45f, 0.0f);
            glScalef(0.18f, legLen * 0.9f, 0.18f);
            glutSolidCube(1.0);
            glPopMatrix();

            // White side stripe
            Renderer::setMaterial(wr, wg, wb, 20.0f);
            glPushMatrix();
            glTranslatef(side * 0.095f, -legLen * 0.45f, 0.0f);
            glScalef(0.02f, legLen * 0.9f, 0.06f);
            glutSolidCube(1.0);
            glPopMatrix();

            // White Shoe with rubber sole
            Renderer::setMaterial(wr, wg, wb, 35.0f);
            glPushMatrix();
            glTranslatef(0.0f, -legLen + 0.04f, 0.04f);
            glScalef(0.19f, 0.12f, 0.28f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Shoe sole
            Renderer::setMaterial(0.20f, 0.20f, 0.22f, 30.0f);
            glPushMatrix();
            glTranslatef(0.0f, -legLen + 0.01f, 0.04f);
            glScalef(0.20f, 0.03f, 0.29f);
            glutSolidCube(1.0);
            glPopMatrix();

            glPopMatrix(); // End Leg
        }

        // 2. TORSO (Squid Game Teal Jacket with Front Zipper & White Number Patches)
        const float torsoH = 0.72f;
        const float torsoW = 0.52f;
        const float torsoD = 0.30f;
        const float shoulderY = hipY + torsoH;

        glPushMatrix();
        glTranslatef(0.0f, hipY + torsoH * 0.5f, 0.0f);

        // Jacket Torso
        Renderer::setMaterial(tr, tg, tb, 25.0f);
        glPushMatrix();
        glScalef(torsoW, torsoH, torsoD);
        glutSolidCube(1.0);
        glPopMatrix();

        // White Front Zipper
        Renderer::setMaterial(wr, wg, wb, 30.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, torsoD * 0.52f);
        glScalef(0.03f, torsoH * 0.95f, 0.02f);
        glutSolidCube(1.0);
        glPopMatrix();

        // White Shoulder Epaulets / Stripes
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(side * 0.18f, torsoH * 0.38f, 0.0f);
            glScalef(0.14f, 0.04f, torsoD * 1.02f);
            glutSolidCube(1.0);
            glPopMatrix();
        }

        // --- WHITE CHEST BADGE WITH CONTESTANT NUMBER ---
        Renderer::setMaterial(wr, wg, wb, 20.0f);
        glPushMatrix();
        glTranslatef(-0.12f, torsoH * 0.12f, torsoD * 0.53f);
        // White patch rectangle
        glPushMatrix();
        glScalef(0.18f, 0.11f, 0.02f);
        glutSolidCube(1.0);
        glPopMatrix();
        // Printed 3-digit number (e.g. 456, 067, 218, 001)
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.015f);
        draw3DigitNumber(number, 0.045f);
        glPopMatrix();
        glPopMatrix();

        // --- WHITE BACK BADGE WITH CONTESTANT NUMBER ---
        Renderer::setMaterial(wr, wg, wb, 20.0f);
        glPushMatrix();
        glTranslatef(0.0f, torsoH * 0.10f, -torsoD * 0.53f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        // White back patch
        glPushMatrix();
        glScalef(0.24f, 0.14f, 0.02f);
        glutSolidCube(1.0);
        glPopMatrix();
        // Printed 3-digit number on back
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.015f);
        draw3DigitNumber(number, 0.060f);
        glPopMatrix();
        glPopMatrix();

        glPopMatrix(); // End Torso

        // 3. ARMS (Teal Sleeves with White Racing Stripes & Skin Hands)
        const float armLen = 0.58f;
        const float armW = 0.15f;

        for (int side = -1; side <= 1; side += 2) {
            float ax = side * (torsoW * 0.5f + armW * 0.5f);
            float aAngle = (side == -1) ? armAngle : -armAngle;

            glPushMatrix();
            glTranslatef(ax, shoulderY, 0.0f); // Shoulder pivot
            glRotatef(aAngle, 1.0f, 0.0f, 0.0f);

            // Sleeve
            Renderer::setMaterial(tr, tg, tb, 25.0f);
            glPushMatrix();
            glTranslatef(0.0f, -armLen * 0.42f, 0.0f);
            glScalef(armW, armLen * 0.84f, armW);
            glutSolidCube(1.0);
            glPopMatrix();

            // White sleeve stripe
            Renderer::setMaterial(wr, wg, wb, 20.0f);
            glPushMatrix();
            glTranslatef(side * 0.075f, -armLen * 0.42f, 0.0f);
            glScalef(0.02f, armLen * 0.84f, 0.05f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Skin Hand
            Renderer::setMaterial(0.92f, 0.74f, 0.60f, 20.0f);
            glPushMatrix();
            glTranslatef(0.0f, -armLen + 0.04f, 0.0f);
            glutSolidSphere(0.07, 10, 10);
            glPopMatrix();

            glPopMatrix(); // End Arm
        }

        // 4. HEAD (Skin Tone + Dark Hair + Collar)
        const float headRadius = 0.26f;

        glPushMatrix();
        glTranslatef(0.0f, shoulderY + headRadius + 0.04f, 0.0f);

        // White Collar Rim
        Renderer::setMaterial(wr, wg, wb, 25.0f);
        glPushMatrix();
        glTranslatef(0.0f, -headRadius * 0.85f, 0.0f);
        glScalef(0.24f, 0.06f, 0.24f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Skin Tone Face
        Renderer::setMaterial(0.92f, 0.74f, 0.60f, 25.0f);
        glutSolidSphere(headRadius, 18, 18);

        // Dark Hair Cap
        Renderer::setMaterial(0.12f, 0.10f, 0.08f, 15.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.06f, -0.03f);
        glScalef(1.02f, 0.95f, 1.02f);
        glutSolidSphere(headRadius, 16, 16);
        glPopMatrix();

        glPopMatrix(); // End Head

        glPopMatrix();
    }
}

void ContestantsManager::init(Player& player, std::vector<NPCContestant>& npcs) {
    // Player 456 (Seong Gi-hun)
    player.x = 0.0f;
    player.y = 0.0f;
    player.z = START_Z;
    player.facingAngle = 0.0f;
    player.legAngle = 0.0f;
    player.armAngle = 0.0f;
    player.walkTime = 0.0f;
    player.isAlive = true;
    player.deathTimer = 0.0f;
    player.playerNumber = 456;
    player.speedTier = SPEED_JOG;
    player.currentSpeed = SPEED_EASY_RUN;

    // NPC Contestants lined up together across the wide starting line
    npcs.clear();
    const int knownNumbers[] = { 67, 218, 1, 199, 101, 212, 240, 324, 250, 70, 118, 96, 62, 303, 111, 453, 360, 276, 130, 244, 388, 196, 83, 306, 17, 40, 69, 99, 100, 222, 333, 444 };

    for (int i = 0; i < NUM_NPC_CONTESTANTS; ++i) {
        NPCContestant npc;
        float laneX = -ARENA_HALF_WIDTH + 3.0f + ((float)i / (NUM_NPC_CONTESTANTS - 1)) * (ARENA_HALF_WIDTH * 2.0f - 6.0f);
        if (fabs(laneX) < 0.8f) {
            laneX = (laneX < 0.0f) ? -1.1f : 1.1f;
        }
        float offsetZ = ((rand() % 20) / 10.0f) - 1.0f;

        npc.x = laneX;
        npc.y = 0.0f;
        npc.z = START_Z + offsetZ;
        npc.facingAngle = 0.0f;
        npc.legAngle = 0.0f;
        npc.armAngle = 0.0f;
        npc.walkTime = 0.0f;
        npc.speed = 0.040f + ((rand() % 22) / 1000.0f); // moderate human speeds
        npc.number = knownNumbers[i % 32];
        npc.isAlive = true;
        npc.fallAngle = 0.0f;
        npc.reactionDelay = 0.08f + ((rand() % 20) / 100.0f);
        npc.willMakeMistake = (rand() % 100 < 25); // ~25% chance of making a fatal error
        npc.mistakeTriggerTime = 0.6f + ((rand() % 240) / 100.0f); // Staggered kill time in Red Light
        npc.hasMovedDuringRed = false;

        npcs.push_back(npc);
    }
}

void ContestantsManager::update(Player& player, std::vector<NPCContestant>& npcs,
                               std::vector<Guard>& guards, const Doll& doll,
                               std::vector<Particle>& particles,
                               int& aliveCount, float dt) {
    bool isRed = (doll.lightState == RED_LIGHT);

    for (auto& npc : npcs) {
        if (!npc.isAlive) {
            // Tumble to floor
            if (npc.fallAngle < 90.0f) {
                npc.fallAngle += 280.0f * dt;
                if (npc.fallAngle > 90.0f) npc.fallAngle = 90.0f;
            }
            continue;
        }

        // Check if finished
        if (npc.z >= FINISH_Z) {
            npc.legAngle = 0.0f;
            npc.armAngle = 0.0f;
            continue;
        }

        bool shouldMove = false;
        if (!isRed) {
            // Green Light: Reset red-light mistake state & advance
            npc.hasMovedDuringRed = false;
            shouldMove = true;
        } else {
            // Red Light: Staggered elimination checks
            if (doll.headAngle > 100.0f) {
                if (npc.willMakeMistake && !npc.hasMovedDuringRed) {
                    npc.mistakeTriggerTime -= dt;
                    if (npc.mistakeTriggerTime <= 0.0f) {
                        // This NPC flinches and gets eliminated!
                        npc.isAlive = false;
                        npc.hasMovedDuringRed = true;
                        aliveCount--;
                        GuardsManager::triggerShot(guards, npc.x, npc.z);
                        ContestantsManager::spawnBloodParticles(particles, npc.x, npc.y, npc.z);
                        AudioManager::playEliminationSound();
                    }
                }
            }
        }

        if (shouldMove && npc.isAlive) {
            npc.z += npc.speed;
            npc.walkTime += dt;
            npc.legAngle = WALK_AMPLITUDE * sinf(WALK_OMEGA * npc.walkTime);
            npc.armAngle = -npc.legAngle;
        } else {
            npc.legAngle = 0.0f;
            npc.armAngle = 0.0f;
        }
    }
}

void ContestantsManager::drawPlayer(const Player& player) {
    glPushMatrix();
    glTranslatef(player.x, player.y, player.z);
    glRotatef(player.facingAngle, 0.0f, 1.0f, 0.0f);
    glScalef(1.15f, 1.15f, 1.15f);

    drawContestantFigure(player.legAngle, player.armAngle, player.playerNumber, player.isAlive, player.deathTimer * 90.0f);

    glPopMatrix();
}

void ContestantsManager::drawNPCs(const std::vector<NPCContestant>& npcs) {
    for (const auto& npc : npcs) {
        glPushMatrix();
        glTranslatef(npc.x, npc.y, npc.z);
        glRotatef(npc.facingAngle, 0.0f, 1.0f, 0.0f);
        glScalef(1.10f, 1.10f, 1.10f);

        drawContestantFigure(npc.legAngle, npc.armAngle, npc.number, npc.isAlive, npc.fallAngle);

        glPopMatrix();
    }
}

void ContestantsManager::spawnBloodParticles(std::vector<Particle>& particles, float x, float y, float z) {
    for (int i = 0; i < 30; ++i) {
        Particle p;
        p.x = x;
        p.y = y + 0.9f;
        p.z = z;
        float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;
        float speed = 1.2f + ((float)rand() / RAND_MAX) * 2.8f;
        p.vx = cosf(angle) * speed;
        p.vy = 1.8f + ((float)rand() / RAND_MAX) * 2.2f;
        p.vz = sinf(angle) * speed;
        p.r = 0.85f + ((float)rand() / RAND_MAX) * 0.15f;
        p.g = 0.02f;
        p.b = 0.02f;
        p.a = 0.95f;
        p.size = 0.08f + ((float)rand() / RAND_MAX) * 0.06f;
        p.life = 0.0f;
        p.maxLife = 0.8f + ((float)rand() / RAND_MAX) * 0.6f;
        particles.push_back(p);
    }
}

void ContestantsManager::updateParticles(std::vector<Particle>& particles, float dt) {
    for (size_t i = 0; i < particles.size();) {
        particles[i].life += dt;
        if (particles[i].life >= particles[i].maxLife) {
            particles.erase(particles.begin() + i);
            continue;
        }

        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        particles[i].z += particles[i].vz * dt;
        particles[i].vy -= 9.8f * dt;

        if (particles[i].y < 0.02f) {
            particles[i].y = 0.02f;
            particles[i].vx *= 0.5f;
            particles[i].vz *= 0.5f;
        }

        particles[i].a = 1.0f - (particles[i].life / particles[i].maxLife);
        ++i;
    }
}

void ContestantsManager::drawParticles(const std::vector<Particle>& particles) {
    if (particles.empty()) return;

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& p : particles) {
        glColor4f(p.r, p.g, p.b, p.a);
        glPushMatrix();
        glTranslatef(p.x, p.y, p.z);
        Renderer::drawSphere(p.size, 8, 8);
        glPopMatrix();
    }

    glPopAttrib();
}
