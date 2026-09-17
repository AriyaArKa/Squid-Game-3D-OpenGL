#include "Environment.h"
#include "Renderer.h"
#include "Config.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdio>

namespace {
    const float PI = 3.14159265358979323846f;

    // Helper to draw a digital 7-segment character in 3D
    void draw7SegmentDigit3D(int digit, float size) {
        static const bool segMap[10][7] = {
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

        if (digit < 0 || digit > 9) return;

        float w = size * 0.45f;
        float h = size * 0.48f;
        float th = size * 0.08f;

        glPushMatrix();

        // 0: Top
        if (segMap[digit][0]) {
            glPushMatrix(); glTranslatef(0.0f, h, 0.0f); Renderer::drawBox(w, th, 0.02f); glPopMatrix();
        }
        // 1: Top-Right
        if (segMap[digit][1]) {
            glPushMatrix(); glTranslatef(w * 0.5f, h * 0.5f, 0.0f); Renderer::drawBox(th, h, 0.02f); glPopMatrix();
        }
        // 2: Bottom-Right
        if (segMap[digit][2]) {
            glPushMatrix(); glTranslatef(w * 0.5f, -h * 0.5f, 0.0f); Renderer::drawBox(th, h, 0.02f); glPopMatrix();
        }
        // 3: Bottom
        if (segMap[digit][3]) {
            glPushMatrix(); glTranslatef(0.0f, -h, 0.0f); Renderer::drawBox(w, th, 0.02f); glPopMatrix();
        }
        // 4: Bottom-Left
        if (segMap[digit][4]) {
            glPushMatrix(); glTranslatef(-w * 0.5f, -h * 0.5f, 0.0f); Renderer::drawBox(th, h, 0.02f); glPopMatrix();
        }
        // 5: Top-Left
        if (segMap[digit][5]) {
            glPushMatrix(); glTranslatef(-w * 0.5f, h * 0.5f, 0.0f); Renderer::drawBox(th, h, 0.02f); glPopMatrix();
        }
        // 6: Middle
        if (segMap[digit][6]) {
            glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f); Renderer::drawBox(w, th, 0.02f); glPopMatrix();
        }

        glPopMatrix();
    }

    // Draw the Squid Game Geometric Symbols (○ △ □) on the house transom panel (Reference Image 4)
    void drawHouseSymbols(float w, float h) {
        Renderer::setMaterial(0.92f, 0.35f, 0.55f, 30.0f); // Hot pink symbol color

        // Circle ○
        glPushMatrix();
        glTranslatef(-w * 0.30f, 0.0f, 0.015f);
        Renderer::drawTorus(0.015f, 0.07f, 8, 16);
        glPopMatrix();

        // Triangle △
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.015f);
        glBegin(GL_LINE_LOOP);
        glLineWidth(2.5f);
        glVertex3f( 0.0f,   0.08f, 0.0f);
        glVertex3f(-0.07f, -0.06f, 0.0f);
        glVertex3f( 0.07f, -0.06f, 0.0f);
        glEnd();
        glPopMatrix();

        // Square □
        glPushMatrix();
        glTranslatef(w * 0.30f, 0.0f, 0.015f);
        glBegin(GL_LINE_LOOP);
        glLineWidth(2.5f);
        glVertex3f(-0.07f,  0.07f, 0.0f);
        glVertex3f( 0.07f,  0.07f, 0.0f);
        glVertex3f( 0.07f, -0.07f, 0.0f);
        glVertex3f(-0.07f, -0.07f, 0.0f);
        glEnd();
        glPopMatrix();
    }

    // Draw an authentic Korean Village House as seen in Reference Images 1, 2, 4
    void drawAuthenticVillageHouse(float x, float z, float rotY) {
        glPushMatrix();
        glTranslatef(x, 0.0f, z);
        glRotatef(rotY, 0.0f, 1.0f, 0.0f);

        // 1. White / Pale-Blue Stucco Building Body
        Renderer::setMaterial(0.88f, 0.90f, 0.93f, 15.0f);
        glPushMatrix();
        glTranslatef(0.0f, 1.6f, 0.0f);
        Renderer::drawBox(4.4f, 3.2f, 3.2f);
        glPopMatrix();

        // 2. Korean Traditional Orange / Terracotta Tiled Roof with Overhang
        Renderer::setMaterial(0.88f, 0.40f, 0.14f, 30.0f);
        glPushMatrix();
        glTranslatef(0.0f, 3.6f, 0.0f);

        // Sloped Hip/Gable Roof
        glBegin(GL_TRIANGLES);
        // Left gable
        glNormal3f(-0.707f, 0.707f, 0.0f);
        glVertex3f(-2.45f, -0.4f, -1.85f);
        glVertex3f(-2.45f, -0.4f,  1.85f);
        glVertex3f(-2.45f,  1.0f,  0.0f);
        // Right gable
        glNormal3f(0.707f, 0.707f, 0.0f);
        glVertex3f(2.45f, -0.4f,  1.85f);
        glVertex3f(2.45f, -0.4f, -1.85f);
        glVertex3f(2.45f,  1.0f,  0.0f);
        glEnd();

        // Front & Back slopes
        glBegin(GL_QUADS);
        // Front slope (facing entrance)
        glNormal3f(0.0f, 0.707f, 0.707f);
        glVertex3f(-2.5f, -0.4f,  1.85f);
        glVertex3f( 2.5f, -0.4f,  1.85f);
        glVertex3f( 2.5f,  1.0f,  0.0f);
        glVertex3f(-2.5f,  1.0f,  0.0f);
        // Back slope
        glNormal3f(0.0f, 0.707f, -0.707f);
        glVertex3f( 2.5f, -0.4f, -1.85f);
        glVertex3f(-2.5f, -0.4f, -1.85f);
        glVertex3f(-2.5f,  1.0f,  0.0f);
        glVertex3f( 2.5f,  1.0f,  0.0f);
        glEnd();

        // White Ridge Tile Crest atop the roof
        Renderer::setMaterial(0.92f, 0.92f, 0.92f, 20.0f);
        glPushMatrix();
        glTranslatef(0.0f, 1.05f, 0.0f);
        Renderer::drawBox(5.0f, 0.14f, 0.18f);
        glPopMatrix();

        glPopMatrix(); // End Roof

        // 3. Rectangular Chimney Stack (Images 1, 2, 4)
        Renderer::setMaterial(0.82f, 0.84f, 0.86f, 15.0f);
        glPushMatrix();
        glTranslatef(1.3f, 4.4f, 0.2f);
        Renderer::drawBox(0.65f, 1.4f, 0.65f);
        // Chimney Cap
        glTranslatef(0.0f, 0.75f, 0.0f);
        Renderer::drawBox(0.80f, 0.12f, 0.80f);
        glPopMatrix();

        // 4. Green Metal Security Double Gate Entrance (Front wall z = +1.61f)
        float frontZ = 1.62f;

        // Dark Green Door Outer Frame
        Renderer::setMaterial(0.10f, 0.38f, 0.24f, 35.0f);
        glPushMatrix();
        glTranslatef(-0.55f, 1.35f, frontZ);
        Renderer::drawBox(1.95f, 2.45f, 0.08f);
        glPopMatrix();

        // Door Paneling & Vertical Security Slats (Reference Image 1)
        Renderer::setMaterial(0.12f, 0.48f, 0.30f, 45.0f);
        for (float sx = -1.35f; sx <= 0.25f; sx += 0.14f) {
            glPushMatrix();
            glTranslatef(sx, 1.25f, frontZ + 0.04f);
            Renderer::drawBox(0.04f, 2.0f, 0.03f);
            glPopMatrix();
        }

        // Transom Header Panel above the Gate with ○ △ □ Symbols (Image 4!)
        Renderer::setMaterial(0.08f, 0.28f, 0.18f, 40.0f);
        glPushMatrix();
        glTranslatef(-0.55f, 2.35f, frontZ + 0.04f);
        Renderer::drawBox(1.85f, 0.35f, 0.04f);
        // Symbols
        drawHouseSymbols(1.8f, 0.3f);
        glPopMatrix();

        // Twin Brass / Iron Door Rings
        Renderer::setMaterial(0.85f, 0.75f, 0.25f, 60.0f);
        glPushMatrix();
        glTranslatef(-0.65f, 1.30f, frontZ + 0.06f);
        Renderer::drawTorus(0.015f, 0.05f, 8, 12);
        glTranslatef(0.20f, 0.0f, 0.0f);
        Renderer::drawTorus(0.015f, 0.05f, 8, 12);
        glPopMatrix();

        // 5. Arched Green Window with Panes on the Right Side (Reference Images 1, 4)
        Renderer::setMaterial(0.10f, 0.40f, 0.25f, 40.0f);
        glPushMatrix();
        glTranslatef(1.15f, 1.8f, frontZ + 0.02f);
        // Lower window box
        Renderer::drawBox(0.95f, 1.15f, 0.04f);
        // Arched top
        glPushMatrix();
        glTranslatef(0.0f, 0.58f, 0.0f);
        glScalef(0.48f, 0.38f, 0.04f);
        glutSolidSphere(1.0, 14, 14);
        glPopMatrix();

        // Window Mullions / Panes (Dark Green)
        Renderer::setMaterial(0.06f, 0.24f, 0.15f, 30.0f);
        // Vertical mullion
        glPushMatrix(); glTranslatef(0.0f, 0.08f, 0.03f); Renderer::drawBox(0.04f, 1.35f, 0.03f); glPopMatrix();
        // Horizontal mullion
        glPushMatrix(); glTranslatef(0.0f, 0.15f, 0.03f); Renderer::drawBox(0.90f, 0.04f, 0.03f); glPopMatrix();
        glPopMatrix();

        glPopMatrix();
    }

    // Draw a cluster of fluffy volumetric clouds on wall murals (Reference Images 1, 2, 3)
    void drawPaintedCloudCluster(float cx, float cy, float cz, float scale, bool alongZWall) {
        glPushMatrix();
        glTranslatef(cx, cy, cz);

        // Fluffy cumulus cloud spheres
        Renderer::setMaterial(0.98f, 0.98f, 1.0f, 10.0f); // Bright pure white cloud top

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
    drawArenaGround();
    drawPaintedSkyWalls();
    drawWheatFieldTrim();
    drawBackgroundHouses(); // Only at player starting side; NO houses at doll side!
    drawBoundaryLines();
    drawOverheadDigitalClock(gameTimer);
}

void EnvironmentManager::drawArenaGround() {
    // Multi-tiled sandy dirt floor with warm golden earth tones (Reference Images 1, 2, 3)
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

            // Subtle warm sand color variation
            float noise = sinf(x1 * 0.8f) * cosf(z1 * 0.6f) * 0.035f;
            float r = 0.89f + noise;
            float g = 0.77f + noise * 0.8f;
            float b = 0.58f + noise * 0.6f;

            glColor3f(r, g, b);
            glVertex3f(x1, 0.0f, z1);
            glVertex3f(x2, 0.0f, z1);
            glVertex3f(x2, 0.0f, z2);
            glVertex3f(x1, 0.0f, z2);
        }
    }
    glEnd();

    glPopMatrix();
}

void EnvironmentManager::drawPaintedSkyWalls() {
    // Giant walls enclosing the arena painted like the sky with clouds (Reference Images 1, 2, 3)
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

    const float hw = ARENA_HALF_WIDTH;
    const float hl = ARENA_HALF_LENGTH;
    const float h  = WALL_HEIGHT;

    // Sky colors: Soft, pleasant storybook blue gradient
    const float skyTopR = 0.40f, skyTopG = 0.70f, skyTopB = 0.94f;
    const float skyBotR = 0.78f, skyBotG = 0.89f, skyBotB = 0.98f;

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
    Renderer::setMaterial(0.86f, 0.70f, 0.24f, 15.0f);

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
    Renderer::setMaterial(0.38f, 0.54f, 0.22f, 20.0f);

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

void EnvironmentManager::drawBackgroundHouses() {
    // =========================================================================
    // HOUSES ONLY AT PLAYER START BACKGROUND (Reference Images 1 & 2)
    // THERE ARE NO HOUSES AT THE DOLL SIDE! (as explicitly requested)
    // =========================================================================

    // 1. Row of 5 Authentic Village Houses lined up behind all 456 contestants (Image 2!)
    // Back wall is at z = -ARENA_HALF_LENGTH (-75). Houses stand at z = -68.0f facing forward (+Z).
    drawAuthenticVillageHouse(-17.5f, -67.0f, 0.0f);
    drawAuthenticVillageHouse( -8.8f, -67.0f, 0.0f);
    drawAuthenticVillageHouse(  0.0f, -67.0f, 0.0f);
    drawAuthenticVillageHouse(  8.8f, -67.0f, 0.0f);
    drawAuthenticVillageHouse( 17.5f, -67.0f, 0.0f);

    // 2. Side Houses along Left & Right Walls near the start line (Reference Image 1!)
    // Left Wall houses facing into the arena (+X, rotY = 90)
    drawAuthenticVillageHouse(-ARENA_HALF_WIDTH + 2.0f, -56.0f, 90.0f);
    drawAuthenticVillageHouse(-ARENA_HALF_WIDTH + 2.0f, -44.0f, 90.0f);

    // Right Wall houses facing into the arena (-X, rotY = -90)
    drawAuthenticVillageHouse( ARENA_HALF_WIDTH - 2.0f, -56.0f, -90.0f);
    drawAuthenticVillageHouse( ARENA_HALF_WIDTH - 2.0f, -44.0f, -90.0f);
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
    Renderer::setMaterial(0.96f, 0.18f, 0.52f, 50.0f);
    Renderer::setEmissive(0.95f, 0.12f, 0.45f, 0.5f);
    glPushMatrix();
    glTranslatef(0.0f, 0.01f, FINISH_Z);
    glScalef(ARENA_HALF_WIDTH * 2.0f, 0.01f, 0.45f);
    glutSolidCube(1.0);
    glPopMatrix();
    Renderer::resetEmission();
}

void EnvironmentManager::drawOverheadDigitalClock(float remainingSeconds) {
    if (remainingSeconds < 0.0f) remainingSeconds = 0.0f;
    int totalSec = (int)remainingSeconds;
    int mins = totalSec / 60;
    int secs = totalSec % 60;

    int d1 = mins / 10;
    int d2 = mins % 10;
    int d3 = secs / 10;
    int d4 = secs % 10;

    glPushMatrix();
    glTranslatef(0.0f, 17.5f, 42.0f);
    glScalef(2.0f, 2.0f, 2.0f);

    // 1. Heavy Black Industrial Outer Frame
    Renderer::setMaterial(0.08f, 0.08f, 0.09f, 50.0f);
    glPushMatrix();
    Renderer::drawBox(5.6f, 2.4f, 0.6f);
    glPopMatrix();

    // 2. Cyan Glowing Scoreboard Border
    Renderer::setMaterial(0.15f, 0.85f, 0.95f, 80.0f);
    Renderer::setEmissive(0.1f, 0.9f, 1.0f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.28f);
    glPushMatrix(); glTranslatef(0.0f, 0.95f, 0.0f); Renderer::drawBox(5.0f, 0.08f, 0.06f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, -0.95f, 0.0f); Renderer::drawBox(5.0f, 0.08f, 0.06f); glPopMatrix();
    glPushMatrix(); glTranslatef(-2.45f, 0.0f, 0.0f); Renderer::drawBox(0.08f, 1.9f, 0.06f); glPopMatrix();
    glPushMatrix(); glTranslatef(2.45f, 0.0f, 0.0f); Renderer::drawBox(0.08f, 1.9f, 0.06f); glPopMatrix();
    glPopMatrix();
    Renderer::resetEmission();

    // 3. Black Inner Display Face
    Renderer::setMaterial(0.04f, 0.04f, 0.05f, 20.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.29f);
    Renderer::drawBox(4.8f, 1.8f, 0.04f);
    glPopMatrix();

    // 4. Glowing Red Digital 7-Segment Digits: "05:00"
    Renderer::setMaterial(1.0f, 0.15f, 0.15f, 100.0f);
    Renderer::setEmissive(1.0f, 0.08f, 0.08f, 1.0f);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.33f);

    float digitSize = 1.35f;
    glPushMatrix(); glTranslatef(-1.65f, 0.0f, 0.0f); draw7SegmentDigit3D(d1, digitSize); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.60f, 0.0f, 0.0f); draw7SegmentDigit3D(d2, digitSize); glPopMatrix();

    // Colon dots ":"
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f); Renderer::drawBox(0.12f, 0.12f, 0.02f);
    glTranslatef(0.0f, -0.50f, 0.0f); Renderer::drawBox(0.12f, 0.12f, 0.02f);
    glPopMatrix();

    glPushMatrix(); glTranslatef(0.60f, 0.0f, 0.0f); draw7SegmentDigit3D(d3, digitSize); glPopMatrix();
    glPushMatrix(); glTranslatef(1.65f, 0.0f, 0.0f); draw7SegmentDigit3D(d4, digitSize); glPopMatrix();

    glPopMatrix();
    Renderer::resetEmission();

    glPopMatrix();
}
