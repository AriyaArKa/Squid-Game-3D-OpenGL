#include "glassbridge/GlassBridge.h"
#include "engine/GLCompat.h"
#include <GL/glut.h>
#include <cmath>

namespace {
    // 3D digit segment rendering for bibs
    void drawGBDigit3D(int d, float size) {
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
        float w = size * 0.44f;
        float h = size * 0.46f;
        float t = size * 0.11f;

        if (segs[d][0]) { glPushMatrix(); glTranslatef(0.0f, h, 0.0f); GLCompat::drawBox(w, t, 0.015f); glPopMatrix(); }
        if (segs[d][1]) { glPushMatrix(); glTranslatef(w*0.5f, h*0.5f, 0.0f); GLCompat::drawBox(t, h, 0.015f); glPopMatrix(); }
        if (segs[d][2]) { glPushMatrix(); glTranslatef(w*0.5f, -h*0.5f, 0.0f); GLCompat::drawBox(t, h, 0.015f); glPopMatrix(); }
        if (segs[d][3]) { glPushMatrix(); glTranslatef(0.0f, -h, 0.0f); GLCompat::drawBox(w, t, 0.015f); glPopMatrix(); }
        if (segs[d][4]) { glPushMatrix(); glTranslatef(-w*0.5f, -h*0.5f, 0.0f); GLCompat::drawBox(t, h, 0.015f); glPopMatrix(); }
        if (segs[d][5]) { glPushMatrix(); glTranslatef(-w*0.5f, h*0.5f, 0.0f); GLCompat::drawBox(t, h, 0.015f); glPopMatrix(); }
        if (segs[d][6]) { glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f); GLCompat::drawBox(w, t, 0.015f); glPopMatrix(); }
    }

    void draw2DigitQueueNumber(int num, float size) {
        int d1 = (num / 10) % 10;
        int d2 = num % 10;
        float spacing = size * 0.54f;
        glPushMatrix(); glTranslatef(-spacing, 0.0f, 0.0f); drawGBDigit3D(d1, size); glPopMatrix();
        glPushMatrix(); glTranslatef(spacing,  0.0f, 0.0f); drawGBDigit3D(d2, size); glPopMatrix();
    }

    // Draw bib with Squid Game number on front and back
    void drawContestantBib(int queueNum, int bibNum, bool isPlayer) {
        // Front Bib
        glPushMatrix();
        glTranslatef(0.0f, 1.05f, 0.16f);
        if (isPlayer) {
            GLCompat::setMaterial(0.08f, 0.78f, 0.40f, 35.0f); // Bright emerald for Player 456
        } else {
            GLCompat::setMaterial(0.92f, 0.15f, 0.48f, 30.0f); // Hot pink bib
        }
        GLCompat::drawBox(0.28f, 0.22f, 0.02f);

        // Black inner patch
        glTranslatef(0.0f, 0.0f, 0.014f);
        GLCompat::setMaterial(0.08f, 0.08f, 0.10f, 20.0f);
        GLCompat::drawBox(0.22f, 0.16f, 0.01f);

        // Bold white 2-digit queue number on chest
        glTranslatef(0.0f, 0.0f, 0.012f);
        GLCompat::setMaterial(0.98f, 0.98f, 0.98f, 35.0f);
        GLCompat::setEmissive(0.85f, 0.85f, 0.85f, 0.6f);
        draw2DigitQueueNumber(queueNum, 0.072f);
        GLCompat::resetEmission();
        glPopMatrix();

        // Back Bib (Visible when looking from behind like Episode 7)
        glPushMatrix();
        glTranslatef(0.0f, 1.05f, -0.16f);
        if (isPlayer) {
            GLCompat::setMaterial(0.08f, 0.78f, 0.40f, 35.0f);
        } else {
            GLCompat::setMaterial(0.92f, 0.15f, 0.48f, 30.0f);
        }
        GLCompat::drawBox(0.28f, 0.22f, 0.02f);

        // Black inner patch
        glTranslatef(0.0f, 0.0f, -0.014f);
        GLCompat::setMaterial(0.08f, 0.08f, 0.10f, 20.0f);
        GLCompat::drawBox(0.22f, 0.16f, 0.01f);

        // Bold white 2-digit queue number on back
        glTranslatef(0.0f, 0.0f, -0.012f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        GLCompat::setMaterial(0.98f, 0.98f, 0.98f, 35.0f);
        GLCompat::setEmissive(0.85f, 0.85f, 0.85f, 0.6f);
        draw2DigitQueueNumber(queueNum, 0.072f);
        GLCompat::resetEmission();
        glPopMatrix();
    }

    // Draw full human contestant with green tracksuit, limbs, and bib
    void drawGBContestantFigure(const GBContestant& c) {
        glPushMatrix();
        glTranslatef(c.x, c.y, c.z);

        // Tumbling spin during fall
        if (!c.isAlive) {
            glRotatef(c.fallVelocity * 30.0f, 1.0f, 0.3f, 0.5f);
        }

        // Teal / Emerald Tracksuit Color
        float trR = c.isPlayer456 ? 0.05f : 0.08f;
        float trG = c.isPlayer456 ? 0.46f : 0.40f;
        float trB = c.isPlayer456 ? 0.38f : 0.35f;

        // 1. White Slip-on Shoes
        GLCompat::setMaterial(0.90f, 0.90f, 0.90f, 30.0f);
        glPushMatrix();
        glTranslatef(-0.14f, 0.08f, 0.02f);
        GLCompat::drawBox(0.14f, 0.16f, 0.26f);
        glTranslatef(0.28f, 0.0f, 0.0f);
        GLCompat::drawBox(0.14f, 0.16f, 0.26f);
        glPopMatrix();

        // 2. Legs with Tracksuit Pants
        GLCompat::setMaterial(trR, trG, trB, 25.0f);
        glPushMatrix();
        // Left Leg
        glPushMatrix();
        glTranslatef(-0.14f, 0.42f, 0.0f);
        if (c.isJumping) glRotatef(-20.0f, 1.0f, 0.0f, 0.0f);
        GLCompat::drawBox(0.16f, 0.52f, 0.16f);
        // White side stripe
        glTranslatef(-0.085f, 0.0f, 0.0f);
        GLCompat::setMaterial(0.95f, 0.95f, 0.95f, 20.0f);
        GLCompat::drawBox(0.015f, 0.52f, 0.04f);
        glPopMatrix();

        // Right Leg
        GLCompat::setMaterial(trR, trG, trB, 25.0f);
        glPushMatrix();
        glTranslatef(0.14f, 0.42f, 0.0f);
        if (c.isJumping) glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
        GLCompat::drawBox(0.16f, 0.52f, 0.16f);
        // White side stripe
        glTranslatef(0.085f, 0.0f, 0.0f);
        GLCompat::setMaterial(0.95f, 0.95f, 0.95f, 20.0f);
        GLCompat::drawBox(0.015f, 0.52f, 0.04f);
        glPopMatrix();
        glPopMatrix();

        // 3. Torso / Tracksuit Jacket
        GLCompat::setMaterial(trR, trG, trB, 25.0f);
        glPushMatrix();
        glTranslatef(0.0f, 1.02f, 0.0f);
        GLCompat::drawBox(0.44f, 0.68f, 0.26f);

        // White zipper and collar
        GLCompat::setMaterial(0.92f, 0.92f, 0.92f, 30.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.135f);
        GLCompat::drawBox(0.03f, 0.68f, 0.01f);
        glPopMatrix();

        // White sleeve stripes
        glPushMatrix();
        glTranslatef(-0.225f, 0.0f, 0.0f);
        GLCompat::drawBox(0.015f, 0.68f, 0.04f);
        glTranslatef(0.45f, 0.0f, 0.0f);
        GLCompat::drawBox(0.015f, 0.68f, 0.04f);
        glPopMatrix();
        glPopMatrix();

        // 4. Arms
        GLCompat::setMaterial(trR, trG, trB, 25.0f);
        // Left Arm
        glPushMatrix();
        glTranslatef(-0.28f, 0.98f, 0.0f);
        if (c.isJumping) glRotatef(-35.0f, 1.0f, 0.0f, 0.0f);
        GLCompat::drawBox(0.12f, 0.58f, 0.12f);
        // Hand (Skin tone)
        glTranslatef(0.0f, -0.32f, 0.0f);
        GLCompat::setMaterial(0.85f, 0.68f, 0.55f, 15.0f);
        GLCompat::drawSphere(0.06f, 8, 8);
        glPopMatrix();

        // Right Arm
        GLCompat::setMaterial(trR, trG, trB, 25.0f);
        glPushMatrix();
        glTranslatef(0.28f, 0.98f, 0.0f);
        if (c.isJumping) glRotatef(-35.0f, 1.0f, 0.0f, 0.0f);
        GLCompat::drawBox(0.12f, 0.58f, 0.12f);
        // Hand (Skin tone)
        glTranslatef(0.0f, -0.32f, 0.0f);
        GLCompat::setMaterial(0.85f, 0.68f, 0.55f, 15.0f);
        GLCompat::drawSphere(0.06f, 8, 8);
        glPopMatrix();

        // 5. Head & Hair
        glPushMatrix();
        glTranslatef(0.0f, 1.50f, 0.0f);
        // Neck
        GLCompat::setMaterial(0.85f, 0.68f, 0.55f, 15.0f);
        glPushMatrix();
        glTranslatef(0.0f, -0.10f, 0.0f);
        GLCompat::drawCylinder(0.08f, 0.08f, 0.10f, 8);
        glPopMatrix();

        // Head Sphere
        GLCompat::drawSphere(0.18f, 12, 12);

        // Black Hair
        GLCompat::setMaterial(0.06f, 0.06f, 0.07f, 20.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.06f, -0.03f);
        GLCompat::drawSphere(0.185f, 12, 12);
        glPopMatrix();
        glPopMatrix();

        // 6. 3D Front & Back Bib with queue number
        drawContestantBib(c.queueNumber, c.bibNumber, c.isPlayer456);

        glPopMatrix();
    }
}

// -----------------------------------------------------------------------------------
// DRAW PINK SOLDIER GUARD
// -----------------------------------------------------------------------------------
void GlassBridgeManager::drawGuard(float x, float y, float z, float angleY, char maskShape) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    const float pr = 0.88f, pg = 0.10f, pb = 0.40f;
    GLCompat::setMaterial(pr, pg, pb, 20.0f);

    // Boots
    GLCompat::setMaterial(0.08f, 0.08f, 0.10f, 25.0f);
    glPushMatrix();
    glTranslatef(-0.16f, 0.10f, 0.02f);
    GLCompat::drawBox(0.15f, 0.20f, 0.28f);
    glTranslatef(0.32f, 0.0f, 0.0f);
    GLCompat::drawBox(0.15f, 0.20f, 0.28f);
    glPopMatrix();

    // Legs
    GLCompat::setMaterial(pr, pg, pb, 20.0f);
    glPushMatrix();
    glTranslatef(-0.16f, 0.45f, 0.0f);
    GLCompat::drawBox(0.18f, 0.55f, 0.18f);
    glTranslatef(0.32f, 0.0f, 0.0f);
    GLCompat::drawBox(0.18f, 0.55f, 0.18f);
    glPopMatrix();

    // Torso
    glPushMatrix();
    glTranslatef(0.0f, 1.05f, 0.0f);
    GLCompat::drawBox(0.48f, 0.70f, 0.30f);
    glPopMatrix();

    // Black Harness Straps
    GLCompat::setMaterial(0.08f, 0.08f, 0.08f, 30.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.05f, 0.0f);
    GLCompat::drawBox(0.49f, 0.08f, 0.31f);
    GLCompat::drawBox(0.10f, 0.71f, 0.31f);
    glPopMatrix();

    // Arms holding MP5
    glPushMatrix();
    glTranslatef(0.0f, 0.95f, 0.26f);
    GLCompat::setMaterial(0.08f, 0.08f, 0.10f, 40.0f);
    GLCompat::drawBox(0.10f, 0.14f, 0.52f);
    GLCompat::drawBox(0.05f, 0.05f, 0.70f);
    glPopMatrix();

    // Pink Hood
    GLCompat::setMaterial(pr, pg, pb, 20.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.55f, 0.0f);
    GLCompat::drawSphere(0.24f, 12, 12);
    glPopMatrix();

    // Black Mesh Mask
    GLCompat::setMaterial(0.04f, 0.04f, 0.05f, 45.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.54f, 0.14f);
    GLCompat::drawSphere(0.17f, 12, 12);

    // White geometric mask symbol
    GLCompat::setEmissive(1.0f, 1.0f, 1.0f, 1.0f);
    glTranslatef(0.0f, 0.02f, 0.14f);
    if (maskShape == 'C') {
        GLCompat::drawTorus(0.015f, 0.065f, 10, 16);
    } else if (maskShape == 'T') {
        GLCompat::drawBox(0.10f, 0.10f, 0.02f);
    } else {
        GLCompat::drawBox(0.12f, 0.12f, 0.02f);
    }
    GLCompat::resetEmission();
    glPopMatrix();

    glPopMatrix();
}

// -----------------------------------------------------------------------------------
// 3D DRAW: CONTESTANTS
// -----------------------------------------------------------------------------------
void GlassBridgeManager::drawContestants(const std::vector<GBContestant>& contestants, int activeQueueIndex) {
    for (size_t i = 0; i < contestants.size(); ++i) {
        const GBContestant& c = contestants[i];
        if (c.y > -38.0f) {
            drawGBContestantFigure(c);
        }
    }
}
