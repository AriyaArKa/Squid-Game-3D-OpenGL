#include "glassbridge/GlassBridge.h"
#include "engine/GLCompat.h"
#include "render/SceneTextures.h"
#include <GL/glut.h>

// -----------------------------------------------------------------------------------
// GLASS BRIDGE HALL (Episode 7): a tall concrete hall with the bridge hanging in the
// middle, a safety net and a deep pit below, lamps along both side walls, light troughs
// in the ceiling, and a start / finish platform at the two ends.
// -----------------------------------------------------------------------------------
namespace {
    const float HALL_HALF_W  = 20.0f;
    const float HALL_MIN_Z   = -16.0f;
    const float HALL_MAX_Z   =  72.0f;
    const float HALL_LEN     = HALL_MAX_Z - HALL_MIN_Z;
    const float HALL_MID_Z   = (HALL_MIN_Z + HALL_MAX_Z) * 0.5f;
    const float HALL_FLOOR_Y = -14.0f;
    const float HALL_TOP_Y   =  34.0f;
    const float CONCRETE_TILE = 8.0f; // one concrete texture image covers 8 x 8 units

    // Large glowing Squid Game symbol (circle / triangle / square) mounted on a side wall.
    // `side` is -1 for the left wall (x < 0) and +1 for the right wall.
    void drawWallSymbol(char shape, float side, float y, float z, const glm::vec3& color) {
        GLCompat::pushMatrix();
        GLCompat::translatef(side * (HALL_HALF_W - 0.30f), y, z);
        GLCompat::rotatef(side * -90.0f, 0.0f, 1.0f, 0.0f); // face into the hall

        // Dark backing plate so the neon reads clearly against the light concrete
        GLCompat::setMaterial(0.10f, 0.11f, 0.14f, 30.0f, 0.2f);
        GLCompat::pushMatrix();
        GLCompat::translatef(0.0f, 0.0f, -0.08f);
        GLCompat::drawBox(5.6f, 5.6f, 0.10f);
        GLCompat::popMatrix();

        GLCompat::setMaterial(color, 60.0f);
        GLCompat::setEmissive(color.r, color.g, color.b, 1.1f);
        const float tube = 0.16f;
        if (shape == 'C') {
            GLCompat::drawTorus(tube, 2.0f, 12, 40);
        } else if (shape == 'T') {
            // Equilateral triangle from 3 bars (side 4.2): base + two sides tilted by 30 degrees
            const float sideLen = 4.2f;
            const float h = sideLen * 0.866f;
            GLCompat::pushMatrix();
            GLCompat::translatef(0.0f, -h / 3.0f, 0.0f);
            GLCompat::drawBox(sideLen, tube * 2.0f, tube * 2.0f);
            GLCompat::popMatrix();
            for (int s = -1; s <= 1; s += 2) {
                GLCompat::pushMatrix();
                GLCompat::translatef(s * sideLen * 0.25f, h / 6.0f, 0.0f);
                GLCompat::rotatef(s * 30.0f, 0.0f, 0.0f, 1.0f);
                GLCompat::drawBox(tube * 2.0f, sideLen, tube * 2.0f);
                GLCompat::popMatrix();
            }
        } else {
            // Square from 4 bars (side 3.6)
            const float half = 1.8f;
            for (int s = -1; s <= 1; s += 2) {
                GLCompat::pushMatrix();
                GLCompat::translatef(0.0f, s * half, 0.0f);
                GLCompat::drawBox(half * 2.0f + tube * 2.0f, tube * 2.0f, tube * 2.0f);
                GLCompat::popMatrix();
                GLCompat::pushMatrix();
                GLCompat::translatef(s * half, 0.0f, 0.0f);
                GLCompat::drawBox(tube * 2.0f, half * 2.0f, tube * 2.0f);
                GLCompat::popMatrix();
            }
        }
        GLCompat::resetEmission();
        GLCompat::popMatrix();
    }

    // Railings: a top bar along z plus vertical posts every 2 units.
    void drawPlatformRailings(float length) {
        GLCompat::setMaterial(COLOR_GB_PINK_RAIL, 40.0f);
        for (int side = -1; side <= 1; side += 2) {
            float rx = (float)side * 3.5f;
            glPushMatrix();
            glTranslatef(rx, 1.0f, 0.0f);
            GLCompat::drawBox(0.06f, 0.06f, length - 0.2f);
            glPopMatrix();
            for (float pz = -length * 0.5f + 0.5f; pz <= length * 0.5f - 0.4f; pz += 2.0f) {
                glPushMatrix();
                glTranslatef(rx, 0.5f, pz);
                GLCompat::drawBox(0.06f, 1.0f, 0.06f);
                glPopMatrix();
            }
        }
    }

    // Pink doorway arch at the back of a platform. `facing` (+1 / -1) is the z direction
    // the doorway looks toward (toward the bridge). `badge` = 'C' draws a glowing ring.
    void drawArch(float z, float facing, char badge) {
        glPushMatrix();
        glTranslatef(0.0f, 2.5f, z);
        GLCompat::setMaterial(0.92f, 0.16f, 0.48f, 40.0f);
        GLCompat::drawBox(4.4f, 4.6f, 0.35f);              // outer pink frame
        GLCompat::setMaterial(0.05f, 0.05f, 0.07f, 15.0f);
        GLCompat::drawBox(2.6f, 3.8f, 0.42f);              // dark doorway
        if (badge == 'C') {
            GLCompat::setMaterial(0.98f, 0.98f, 0.98f, 80.0f);
            GLCompat::setEmissive(0.98f, 0.98f, 0.98f, 1.2f);
            glPushMatrix();
            glTranslatef(0.0f, 2.05f, facing * 0.24f);
            GLCompat::drawTorus(0.035f, 0.26f, 8, 16);
            glPopMatrix();
        } else {
            GLCompat::setEmissive(0.95f, 0.18f, 0.52f, 1.2f);
            glPushMatrix();
            glTranslatef(0.0f, 1.95f, facing * 0.22f);
            GLCompat::drawBox(2.8f, 0.10f, 0.05f);         // glowing strip over the door
            glPopMatrix();
        }
        GLCompat::resetEmission();
        glPopMatrix();
    }

    // Concrete deck + yellow hazard edge. `edgeZ` is the local z of the jump/landing edge.
    void drawPlatformDeck(float length, float edgeZ) {
        GLCompat::setMaterial(COLOR_GB_PLATFORM, 30.0f, 0.15f);
        GLCompat::setTexture(&SceneTextures::concrete(), 7.2f / 4.0f, length / 4.0f);
        GLCompat::drawBox(7.2f, 0.5f, length);
        GLCompat::clearTexture();

        GLCompat::setMaterial(0.95f, 0.78f, 0.12f, 50.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.27f, edgeZ);
        GLCompat::drawBox(7.2f, 0.06f, 0.25f);
        glPopMatrix();
    }
}

void GlassBridgeManager::drawEnvironment() {
    drawHall();
    drawSafetyNet();
    drawStartPlatform();
    drawFinishPlatform();

    const float destZ = GB_BRIDGE_START_Z + (float)GB_NUM_STEPS * GB_STEP_LENGTH + 5.2f;
    // Two guards at the start ledge, two at the finish exit
    drawGuard(-2.4f, GB_BRIDGE_HEIGHT, 0.2f, 30.0f, 'C');
    drawGuard( 2.4f, GB_BRIDGE_HEIGHT, 0.2f, -30.0f, 'T');
    drawGuard(-2.2f, GB_BRIDGE_HEIGHT, destZ - 2.0f, 150.0f, 'S');
    drawGuard( 2.2f, GB_BRIDGE_HEIGHT, destZ - 2.0f, -150.0f, 'T');
}

// -----------------------------------------------------------------------------------
// 1. THE HALL: concrete walls, ceiling with light troughs, pit floor, wall details
// -----------------------------------------------------------------------------------
void GlassBridgeManager::drawHall() {
    const float bridgeY = GB_BRIDGE_HEIGHT;
    const float wallH = HALL_TOP_Y - HALL_FLOOR_Y;
    const float wallMidY = (HALL_TOP_Y + HALL_FLOOR_Y) * 0.5f;

    // (a) Left and right concrete walls (texture tiled every 8 units)
    GLCompat::setMaterial(COLOR_GB_WALL, 18.0f, 0.08f);
    GLCompat::setTexture(&SceneTextures::concrete(), HALL_LEN / CONCRETE_TILE, wallH / CONCRETE_TILE);
    for (int side = -1; side <= 1; side += 2) {
        GLCompat::pushMatrix();
        GLCompat::translatef(side * HALL_HALF_W, wallMidY, HALL_MID_Z);
        GLCompat::drawBox(0.4f, wallH, HALL_LEN);
        GLCompat::popMatrix();
    }

    // (b) Back wall (behind the start) and front wall (behind the finish)
    GLCompat::setTexture(&SceneTextures::concrete(), HALL_HALF_W * 2.0f / CONCRETE_TILE, wallH / CONCRETE_TILE);
    for (float wz : {HALL_MIN_Z, HALL_MAX_Z}) {
        GLCompat::pushMatrix();
        GLCompat::translatef(0.0f, wallMidY, wz);
        GLCompat::drawBox(HALL_HALF_W * 2.0f, wallH, 0.4f);
        GLCompat::popMatrix();
    }

    // (c) Ceiling slab and pit floor (same concrete, darker tints)
    GLCompat::setTexture(&SceneTextures::concrete(), HALL_HALF_W * 2.0f / CONCRETE_TILE, HALL_LEN / CONCRETE_TILE);
    GLCompat::setMaterial(COLOR_GB_CEILING, 10.0f, 0.05f);
    GLCompat::pushMatrix();
    GLCompat::translatef(0.0f, HALL_TOP_Y, HALL_MID_Z);
    GLCompat::drawBox(HALL_HALF_W * 2.0f, 0.4f, HALL_LEN);
    GLCompat::popMatrix();

    GLCompat::setMaterial(COLOR_GB_ABYSS, 8.0f, 0.05f);
    GLCompat::pushMatrix();
    GLCompat::translatef(0.0f, HALL_FLOOR_Y, HALL_MID_Z);
    GLCompat::drawBox(HALL_HALF_W * 2.0f, 0.5f, HALL_LEN);
    GLCompat::popMatrix();
    GLCompat::clearTexture();

    // (d) Two long light troughs in the ceiling (glowing panels)
    for (int side = -1; side <= 1; side += 2) {
        GLCompat::setMaterial(0.20f, 0.22f, 0.26f, 30.0f);
        GLCompat::pushMatrix();
        GLCompat::translatef(side * 8.0f, HALL_TOP_Y - 0.30f, HALL_MID_Z);
        GLCompat::drawBox(1.6f, 0.25f, HALL_LEN - 6.0f);
        GLCompat::translatef(0.0f, -0.14f, 0.0f);
        GLCompat::setMaterial(COLOR_GB_LAMP, 10.0f);
        GLCompat::setEmissive(COLOR_GB_LAMP.r, COLOR_GB_LAMP.g, COLOR_GB_LAMP.b, 1.4f);
        GLCompat::drawBox(1.2f, 0.06f, HALL_LEN - 7.0f);
        GLCompat::resetEmission();
        GLCompat::popMatrix();
    }

    // (e) Neon accent strip along each wall at bridge height (cyan left, pink right)
    for (int side = -1; side <= 1; side += 2) {
        const glm::vec3 c = (side < 0) ? COLOR_GB_CYAN : COLOR_GB_PINK_RAIL;
        GLCompat::setMaterial(c, 60.0f);
        GLCompat::setEmissive(c.r, c.g, c.b, 0.9f);
        GLCompat::pushMatrix();
        GLCompat::translatef(side * (HALL_HALF_W - 0.22f), bridgeY, HALL_MID_Z);
        GLCompat::drawBox(0.05f, 0.14f, HALL_LEN);
        GLCompat::popMatrix();
        GLCompat::resetEmission();
    }

    // (f) Steel wall ribs every 8 units, each with a tall LED light bar
    for (float rz = HALL_MIN_Z + 7.0f; rz < HALL_MAX_Z; rz += 8.0f) {
        for (int side = -1; side <= 1; side += 2) {
            float x = (float)side * (HALL_HALF_W - 0.24f);
            GLCompat::setMaterial(COLOR_GB_STEEL_BEAM, 48.0f, 0.25f);
            GLCompat::pushMatrix();
            GLCompat::translatef(x, wallMidY, rz);
            GLCompat::drawBox(0.22f, wallH, 0.40f);
            GLCompat::popMatrix();

            const glm::vec3 led = (side < 0) ? COLOR_GB_CYAN : COLOR_GB_PINK_RAIL;
            GLCompat::setMaterial(led, 80.0f);
            GLCompat::setEmissive(led.r, led.g, led.b, 0.85f);
            GLCompat::pushMatrix();
            GLCompat::translatef(x - (float)side * 0.13f, bridgeY + 4.5f, rz);
            GLCompat::drawBox(0.04f, 6.0f, 0.10f);
            GLCompat::popMatrix();
            GLCompat::resetEmission();
        }
    }

    // (g) Big neon symbols high on both side walls
    const char shapes[3] = {'C', 'T', 'S'};
    const float symbolZ[3] = {9.0f, 29.0f, 49.0f};
    for (int i = 0; i < 3; ++i) {
        drawWallSymbol(shapes[i], -1.0f, bridgeY + 9.0f, symbolZ[i], COLOR_GB_PINK_RAIL);
        drawWallSymbol(shapes[i],  1.0f, bridgeY + 9.0f, symbolZ[i], glm::vec3(0.95f, 0.95f, 1.0f));
    }

    // (h) Overhead steel trusses with suspension cables down to the bridge rails
    const float trussY = 28.5f;
    GLCompat::setMaterial(COLOR_GB_STEEL_LIGHT, 52.0f, 0.3f);
    for (float tz = 0.0f; tz <= 56.0f; tz += 14.0f) {
        GLCompat::pushMatrix();
        GLCompat::translatef(0.0f, trussY, tz);
        GLCompat::drawBox(HALL_HALF_W * 2.0f, 0.6f, 0.6f);
        GLCompat::popMatrix();

        for (int side = -1; side <= 1; side += 2) {
            GLCompat::pushMatrix();
            GLCompat::translatef((float)side * 2.75f, bridgeY + 1.1f, tz);
            GLCompat::rotatef(-90.0f, 1.0f, 0.0f, 0.0f); // cylinder axis z -> up
            GLCompat::drawCylinder(0.025f, 0.025f, trussY - bridgeY - 1.1f, 6);
            GLCompat::popMatrix();
        }
    }
}

// -----------------------------------------------------------------------------------
// 2. SAFETY CARGO NET, 14 units below the bridge
// -----------------------------------------------------------------------------------
void GlassBridgeManager::drawSafetyNet() {
    const float netY = 2.0f;
    const float netMinZ = -2.0f;
    const float netMaxZ = 58.0f;
    const float netW = 14.0f;
    const float netMidZ = (netMinZ + netMaxZ) * 0.5f;

    // Thick border cables
    GLCompat::setMaterial(0.55f, 0.56f, 0.60f, 40.0f);
    for (int side = -1; side <= 1; side += 2) {
        GLCompat::pushMatrix();
        GLCompat::translatef(side * netW * 0.5f, netY, netMidZ);
        GLCompat::drawBox(0.08f, 0.08f, netMaxZ - netMinZ);
        GLCompat::popMatrix();
    }

    // Grid: cables along z every 1.4 units, cables across x every 2.2 units
    GLCompat::setMaterial(0.42f, 0.44f, 0.50f, 25.0f);
    for (float nx = -netW * 0.5f + 1.4f; nx < netW * 0.5f; nx += 1.4f) {
        GLCompat::pushMatrix();
        GLCompat::translatef(nx, netY, netMidZ);
        GLCompat::drawBox(0.03f, 0.03f, netMaxZ - netMinZ);
        GLCompat::popMatrix();
    }
    for (float nz = netMinZ; nz <= netMaxZ; nz += 2.2f) {
        GLCompat::pushMatrix();
        GLCompat::translatef(0.0f, netY, nz);
        GLCompat::drawBox(netW, 0.03f, 0.03f);
        GLCompat::popMatrix();
    }
}

// -----------------------------------------------------------------------------------
// 3. START PLATFORM (queue of 16 contestants) and 4. FINISH PLATFORM
// -----------------------------------------------------------------------------------
void GlassBridgeManager::drawStartPlatform() {
    GLCompat::pushMatrix();
    GLCompat::translatef(0.0f, GB_BRIDGE_HEIGHT - 0.25f, -4.5f);
    drawPlatformDeck(9.0f, 4.45f);
    drawPlatformRailings(9.0f);
    drawArch(-4.3f, 1.0f, 'S');
    GLCompat::popMatrix();
}

void GlassBridgeManager::drawFinishPlatform() {
    const float destZ = GB_BRIDGE_START_Z + (float)GB_NUM_STEPS * GB_STEP_LENGTH + 5.2f;
    GLCompat::pushMatrix();
    GLCompat::translatef(0.0f, GB_BRIDGE_HEIGHT - 0.25f, destZ);
    drawPlatformDeck(8.5f, -4.2f);
    drawArch(4.0f, -1.0f, 'C');
    GLCompat::popMatrix();
}
