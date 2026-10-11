#include "environment/Environment.h"
#include "render/Renderer.h"
#include "core/Config.h"
#include "render/SceneTextures.h"
#include <GL/glut.h>
#include <cmath>
#include <glm/glm.hpp>

// This file covers the arena "shell": the sand floor, the painted sky perimeter walls
// (with their cloud clusters), the wheat-field trim, and the start/finish boundary lines.
// The background village houses live in EnvironmentHouses.cpp and the 3D scoreboard clock
// lives in EnvironmentClock.cpp - both declared in Environment.h and implemented in their
// own file so each piece can be read and debugged on its own.

namespace {
    // Draw a cluster of fluffy volumetric clouds on wall murals (Reference Images 1, 2, 3)
    void drawPaintedCloudCluster(float cx, float cy, float cz, float scale, bool alongZWall) {
        glPushMatrix();
        glTranslatef(cx, cy, cz);

        // Fluffy cumulus cloud spheres
        Renderer::setMaterial(COLOR_ARENA_CLOUD, 28.0f);

        const float offsets[6][4] = {
            { 0.0f,   0.0f,  1.10f, 0.95f}, // Center main billow
            {-1.2f,  -0.2f,  0.85f, 0.96f}, // Left lobe
            { 1.25f, -0.15f, 0.88f, 0.96f}, // Right lobe
            {-0.55f,  0.45f, 0.78f, 1.00f}, // Top left puff
            { 0.60f,  0.40f, 0.82f, 1.00f}, // Top right puff
            { 0.0f,  -0.45f, 0.90f, 0.88f}  // Bottom shaded underside
        };

        for (int i = 0; i < 6; ++i) {
            float ox = offsets[i][0] * scale;
            float oy = offsets[i][1] * scale;
            float r  = offsets[i][2] * scale;
            float shade = offsets[i][3];

            glColor3f(shade * 0.98f, shade * 0.98f, shade * 1.0f);

            glPushMatrix();
            if (alongZWall) {
                glTranslatef(ox, oy, 0.0f);
                glScalef(r, r * 0.65f, 0.08f);
            } else {
                glTranslatef(0.0f, oy, ox);
                glScalef(0.08f, r * 0.65f, r);
            }
            glutSolidSphere(1.0, 14, 14);
            glPopMatrix();
        }

        glPopMatrix();
    }
}

void EnvironmentManager::init() {
}

void EnvironmentManager::draw(float gameTimer) {
    drawSkyDome();
    drawArenaGround();
    drawPaintedSkyWalls();
    drawWheatFieldTrim();
    drawBackgroundHouses(); // Only at player starting side; NO houses at doll side!
    drawBoundaryLines();
    drawOverheadDigitalClock(gameTimer);
}

void EnvironmentManager::drawSkyDome() {
    // A huge half-sphere around the arena, colored from a pale horizon to a deep blue zenith,
    // drawn first without lighting, depth writes or haze so everything else covers it.
    if (GLCompat::getInspect().colorByShape) return; // a backdrop, not a modelled object
    const float R = 190.0f;
    const int slices = 32, stacks = 10;
    const float PI = 3.14159265f;

    GLCompat::setUnlit(true);
    GLCompat::setDepthWrite(false);
    GLCompat::setFog(false);
    Renderer::setMaterial(1.0f, 1.0f, 1.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
    for (int i = 0; i < stacks; ++i) {
        float e0 = (float)i / stacks * (PI * 0.5f) - 0.12f;       // start a little below the horizon
        float e1 = (float)(i + 1) / stacks * (PI * 0.5f) - 0.12f;
        float t0 = glm::clamp(e0 / (PI * 0.5f), 0.0f, 1.0f);
        float t1 = glm::clamp(e1 / (PI * 0.5f), 0.0f, 1.0f);
        glm::vec3 c0 = glm::mix(COLOR_SKY_HORIZON, COLOR_SKY_ZENITH, sqrtf(t0));
        glm::vec3 c1 = glm::mix(COLOR_SKY_HORIZON, COLOR_SKY_ZENITH, sqrtf(t1));
        for (int j = 0; j < slices; ++j) {
            float a0 = (float)j / slices * 2.0f * PI;
            float a1 = (float)(j + 1) / slices * 2.0f * PI;
            glNormal3f(0.0f, -1.0f, 0.0f);
            glColor3f(c0.r, c0.g, c0.b); glVertex3f(R * cosf(e0) * cosf(a0), R * sinf(e0), R * cosf(e0) * sinf(a0));
            glColor3f(c0.r, c0.g, c0.b); glVertex3f(R * cosf(e0) * cosf(a1), R * sinf(e0), R * cosf(e0) * sinf(a1));
            glColor3f(c1.r, c1.g, c1.b); glVertex3f(R * cosf(e1) * cosf(a1), R * sinf(e1), R * cosf(e1) * sinf(a1));
            glColor3f(c1.r, c1.g, c1.b); glVertex3f(R * cosf(e1) * cosf(a0), R * sinf(e1), R * cosf(e1) * sinf(a0));
        }
    }
    glEnd();

    // Soft white clouds floating above the arena walls (flattened sphere clusters)
    const float cloudAngles[8] = {20.0f, 62.0f, 105.0f, 150.0f, 196.0f, 240.0f, 285.0f, 330.0f};
    Renderer::setMaterial(1.0f, 1.0f, 1.0f, 1.0f, 0.0f);
    GLCompat::setAlpha(0.92f);
    GLCompat::enableBlend();
    for (int i = 0; i < 8; ++i) {
        float a = glm::radians(cloudAngles[i]);
        float dist = 150.0f, height = 34.0f + 10.0f * (float)(i % 3);
        glPushMatrix();
        glTranslatef(cosf(a) * dist, height, sinf(a) * dist);
        glRotatef(-cloudAngles[i] + 90.0f, 0.0f, 1.0f, 0.0f); // face the arena
        const float puffs[5][3] = {{0, 0, 9}, {-9, -1, 7}, {9, -1.5f, 7.5f}, {-4, 3, 6.5f}, {4.5f, 2.5f, 7}};
        for (const auto& p : puffs) {
            glPushMatrix();
            glTranslatef(p[0], p[1], 0.0f);
            glScalef(p[2], p[2] * 0.55f, p[2] * 0.4f);
            glutSolidSphere(1.0, 12, 12);
            glPopMatrix();
        }
        glPopMatrix();
    }
    GLCompat::disableBlend();
    GLCompat::setAlpha(1.0f);

    GLCompat::setFog(true);
    GLCompat::setDepthWrite(true);
    GLCompat::setUnlit(false);
}

void EnvironmentManager::drawArenaGround() {
    // Sand floor: a 24 x 32 grid of quads with the sand texture tiled every
    // SAND_TILE units (texture coordinate = world position / SAND_TILE, GL_REPEAT wraps it).
    const float SAND_TILE = 6.0f;
    Renderer::setMaterial(COLOR_ARENA_SAND_TINT, 12.0f, 0.06f);
    GLCompat::setTexture(&SceneTextures::sand());
    glPushMatrix();
    glTranslatef(0.0f, -0.05f, 0.0f);

    const int gridX = 24;
    const int gridZ = 32;
    const float stepX = (ARENA_HALF_WIDTH * 2.0f) / gridX;
    const float stepZ = (ARENA_HALF_LENGTH * 2.0f) / gridZ;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < gridX; ++i) {
        float x1 = -ARENA_HALF_WIDTH + i * stepX;
        float x2 = x1 + stepX;

        for (int j = 0; j < gridZ; ++j) {
            float z1 = -ARENA_HALF_LENGTH + j * stepZ;
            float z2 = z1 + stepZ;

            // Gentle large-scale brightness variation on top of the texture
            float shade = 1.0f + sinf(x1 * 0.8f) * cosf(z1 * 0.6f) * 0.04f;
            glColor3f(COLOR_ARENA_SAND_TINT.r * shade, COLOR_ARENA_SAND_TINT.g * shade, COLOR_ARENA_SAND_TINT.b * shade);
            glTexCoord2f(x1 / SAND_TILE, z1 / SAND_TILE); glVertex3f(x1, 0.0f, z1);
            glTexCoord2f(x2 / SAND_TILE, z1 / SAND_TILE); glVertex3f(x2, 0.0f, z1);
            glTexCoord2f(x2 / SAND_TILE, z2 / SAND_TILE); glVertex3f(x2, 0.0f, z2);
            glTexCoord2f(x1 / SAND_TILE, z2 / SAND_TILE); glVertex3f(x1, 0.0f, z2);
        }
    }
    glEnd();
    GLCompat::clearTexture();

    glPopMatrix();
}

void EnvironmentManager::drawPaintedSkyWalls() {
    // Giant walls enclosing the arena painted like the sky with clouds (Reference Images 1, 2, 3, 4)
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    Renderer::setMaterial(COLOR_ARENA_SKY, 8.0f, 0.03f);

    const float hw = ARENA_HALF_WIDTH;
    const float hl = ARENA_HALF_LENGTH;
    const float h  = WALL_HEIGHT;

    // Sky colors: Deep rich atmospheric storybook blue mural (Matching Reference Image 4)
    const float skyTopR = COLOR_ARENA_SKY_TOP.r, skyTopG = COLOR_ARENA_SKY_TOP.g, skyTopB = COLOR_ARENA_SKY_TOP.b;
    const float skyBotR = COLOR_ARENA_SKY.r, skyBotG = COLOR_ARENA_SKY.g, skyBotB = COLOR_ARENA_SKY.b;

    glBegin(GL_QUADS);

    // Far Wall (behind Doll, z = +hl)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glColor3f(skyBotR, skyBotG, skyBotB); glVertex3f(-hw, 0.0f, hl);
    glColor3f(skyBotR, skyBotG, skyBotB); glVertex3f( hw, 0.0f, hl);
    glColor3f(skyTopR, skyTopG, skyTopB); glVertex3f( hw, h,    hl);
    glColor3f(skyTopR, skyTopG, skyTopB); glVertex3f(-hw, h,    hl);

    // Near Wall (behind Players, z = -hl)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glColor3f(skyBotR, skyBotG, skyBotB); glVertex3f( hw, 0.0f, -hl);
    glColor3f(skyBotR, skyBotG, skyBotB); glVertex3f(-hw, 0.0f, -hl);
    glColor3f(skyTopR, skyTopG, skyTopB); glVertex3f(-hw, h,    -hl);
    glColor3f(skyTopR, skyTopG, skyTopB); glVertex3f( hw, h,    -hl);

    // Left Wall (x = -hw)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glColor3f(skyBotR, skyBotG, skyBotB); glVertex3f(-hw, 0.0f, -hl);
    glColor3f(skyBotR, skyBotG, skyBotB); glVertex3f(-hw, 0.0f,  hl);
    glColor3f(skyTopR, skyTopG, skyTopB); glVertex3f(-hw, h,     hl);
    glColor3f(skyTopR, skyTopG, skyTopB); glVertex3f(-hw, h,    -hl);

    // Right Wall (x = +hw)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glColor3f(skyBotR, skyBotG, skyBotB); glVertex3f(hw, 0.0f,  hl);
    glColor3f(skyBotR, skyBotG, skyBotB); glVertex3f(hw, 0.0f, -hl);
    glColor3f(skyTopR, skyTopG, skyTopB); glVertex3f(hw, h,    -hl);
    glColor3f(skyTopR, skyTopG, skyTopB); glVertex3f(hw, h,     hl);

    glEnd();

    // =========================================================================
    // PAINTED FLUFFY CUMULUS CLOUDS ON ALL TOP AND SIDE WALLS (Reference Images 1, 2, 3)
    // =========================================================================

    // 1. Clouds on Far Wall (behind Doll, z = +hl - 0.06f)
    drawPaintedCloudCluster(-16.0f, 15.5f, hl - 0.06f, 1.8f, true);
    drawPaintedCloudCluster( -7.5f, 17.5f, hl - 0.06f, 2.2f, true);
    drawPaintedCloudCluster(  2.0f, 15.0f, hl - 0.06f, 1.7f, true);
    drawPaintedCloudCluster( 10.5f, 18.0f, hl - 0.06f, 2.4f, true);
    drawPaintedCloudCluster( 18.0f, 15.8f, hl - 0.06f, 1.9f, true);

    // 2. Clouds on Near Wall (behind Players, z = -hl + 0.06f)
    drawPaintedCloudCluster(-15.5f, 16.5f, -hl + 0.06f, 2.1f, true);
    drawPaintedCloudCluster( -6.0f, 18.0f, -hl + 0.06f, 2.5f, true);
    drawPaintedCloudCluster(  4.5f, 16.0f, -hl + 0.06f, 2.0f, true);
    drawPaintedCloudCluster( 14.0f, 17.8f, -hl + 0.06f, 2.3f, true);

    // 3. Clouds on Left Wall (x = -hw + 0.06f, spanning z from -hl to +hl)
    drawPaintedCloudCluster(-hw + 0.06f, 16.0f, -55.0f, 2.0f, false);
    drawPaintedCloudCluster(-hw + 0.06f, 18.0f, -35.0f, 2.4f, false);
    drawPaintedCloudCluster(-hw + 0.06f, 15.5f, -15.0f, 1.9f, false);
    drawPaintedCloudCluster(-hw + 0.06f, 17.5f,   5.0f, 2.3f, false);
    drawPaintedCloudCluster(-hw + 0.06f, 16.0f,  25.0f, 2.0f, false);
    drawPaintedCloudCluster(-hw + 0.06f, 18.2f,  45.0f, 2.5f, false);
    drawPaintedCloudCluster(-hw + 0.06f, 16.2f,  62.0f, 2.1f, false);

    // 4. Clouds on Right Wall (x = hw - 0.06f, spanning z from -hl to +hl)
    drawPaintedCloudCluster(hw - 0.06f, 17.0f, -58.0f, 2.2f, false);
    drawPaintedCloudCluster(hw - 0.06f, 15.8f, -38.0f, 2.0f, false);
    drawPaintedCloudCluster(hw - 0.06f, 18.0f, -18.0f, 2.4f, false);
    drawPaintedCloudCluster(hw - 0.06f, 16.2f,   2.0f, 1.8f, false);
    drawPaintedCloudCluster(hw - 0.06f, 17.8f,  22.0f, 2.3f, false);
    drawPaintedCloudCluster(hw - 0.06f, 15.6f,  42.0f, 2.1f, false);
    drawPaintedCloudCluster(hw - 0.06f, 18.0f,  60.0f, 2.4f, false);

    glPopAttrib();
}

void EnvironmentManager::drawWheatFieldTrim() {
    // Painted Golden Reed / Wheat Field along bottom of arena walls (Reference Images 1, 2, 3)
    const float trimH = 4.2f; // Taller realistic mural height
    const float hw = ARENA_HALF_WIDTH;
    const float hl = ARENA_HALF_LENGTH;

    glPushMatrix();

    // 1. Golden Yellow Wheat Field (Bottom layer, y = 0..3.2)
    Renderer::setMaterial(COLOR_ARENA_WHEAT, 22.0f);

    glBegin(GL_QUADS);
    // Far Wall base
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-hw, 0.0f, hl - 0.02f);
    glVertex3f( hw, 0.0f, hl - 0.02f);
    glVertex3f( hw, 3.2f, hl - 0.02f);
    glVertex3f(-hw, 3.2f, hl - 0.02f);

    // Near Wall base
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f( hw, 0.0f, -hl + 0.02f);
    glVertex3f(-hw, 0.0f, -hl + 0.02f);
    glVertex3f(-hw, 3.2f, -hl + 0.02f);
    glVertex3f( hw, 3.2f, -hl + 0.02f);

    // Left Wall base
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-hw + 0.02f, 0.0f, -hl);
    glVertex3f(-hw + 0.02f, 0.0f,  hl);
    glVertex3f(-hw + 0.02f, 3.2f,  hl);
    glVertex3f(-hw + 0.02f, 3.2f, -hl);

    // Right Wall base
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(hw - 0.02f, 0.0f,  hl);
    glVertex3f(hw - 0.02f, 0.0f, -hl);
    glVertex3f(hw - 0.02f, 3.2f, -hl);
    glVertex3f(hw - 0.02f, 3.2f,  hl);
    glEnd();

    // 2. Green Treetop Horizon Line (Upper layer, y = 3.2..4.2, as in Reference Image 1 & 2)
    Renderer::setMaterial(COLOR_ARENA_HORIZON, 26.0f);

    glBegin(GL_QUADS);
    // Far Wall trees
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-hw, 3.2f, hl - 0.025f);
    glVertex3f( hw, 3.2f, hl - 0.025f);
    glVertex3f( hw, trimH, hl - 0.025f);
    glVertex3f(-hw, trimH, hl - 0.025f);

    // Near Wall trees
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f( hw, 3.2f, -hl + 0.025f);
    glVertex3f(-hw, 3.2f, -hl + 0.025f);
    glVertex3f(-hw, trimH, -hl + 0.025f);
    glVertex3f( hw, trimH, -hl + 0.025f);

    // Left Wall trees
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-hw + 0.025f, 3.2f, -hl);
    glVertex3f(-hw + 0.025f, 3.2f,  hl);
    glVertex3f(-hw + 0.025f, trimH,  hl);
    glVertex3f(-hw + 0.025f, trimH, -hl);

    // Right Wall trees
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(hw - 0.025f, 3.2f,  hl);
    glVertex3f(hw - 0.025f, 3.2f, -hl);
    glVertex3f(hw - 0.025f, trimH, -hl);
    glVertex3f(hw - 0.025f, trimH,  hl);
    glEnd();

    glPopMatrix();
}

void EnvironmentManager::drawBoundaryLines() {
    // Starting Red Line (behind contestants at START_Z)
    Renderer::setMaterial(0.95f, 0.15f, 0.15f, 40.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.01f, START_Z);
    glScalef(ARENA_HALF_WIDTH * 2.0f, 0.01f, 0.35f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Finish Hot Pink Line (in front of doll at FINISH_Z, Reference Image 3!)
    Renderer::setMaterial(COLOR_FINISH_LINE, 50.0f);
    Renderer::setEmissive(COLOR_FINISH_LINE.r, COLOR_FINISH_LINE.g, COLOR_FINISH_LINE.b, 0.5f);
    glPushMatrix();
    glTranslatef(0.0f, 0.01f, FINISH_Z);
    glScalef(ARENA_HALF_WIDTH * 2.0f, 0.01f, 0.45f);
    glutSolidCube(1.0);
    glPopMatrix();
    Renderer::resetEmission();
}
