#include "glassbridge/GlassBridge.h"
#include "contestants/Contestants.h"
#include "engine/GLCompat.h"
#include <GL/glut.h>
#include <algorithm>

// Bridge guards remain round-specific. Contestants, including Player 456, are
// rendered by ContestantsManager so character geometry and palette exist once.
void GlassBridgeManager::drawGuard(float x, float y, float z, float angleY, char maskShape) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    GLCompat::setMaterial(COLOR_GUARD_MAGENTA, 20.0f);

    GLCompat::setMaterial(COLOR_GUARD_BLACK, 25.0f);
    glPushMatrix();
    glTranslatef(-0.16f, 0.10f, 0.02f);
    GLCompat::drawBox(0.15f, 0.20f, 0.28f);
    glTranslatef(0.32f, 0.0f, 0.0f);
    GLCompat::drawBox(0.15f, 0.20f, 0.28f);
    glPopMatrix();

    GLCompat::setMaterial(COLOR_GUARD_MAGENTA, 20.0f);
    glPushMatrix();
    glTranslatef(-0.16f, 0.45f, 0.0f);
    GLCompat::drawBox(0.18f, 0.55f, 0.18f);
    glTranslatef(0.32f, 0.0f, 0.0f);
    GLCompat::drawBox(0.18f, 0.55f, 0.18f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 1.05f, 0.0f);
    GLCompat::drawBox(0.48f, 0.70f, 0.30f);
    glPopMatrix();

    GLCompat::setMaterial(COLOR_GUARD_BLACK, 30.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.05f, 0.0f);
    GLCompat::drawBox(0.49f, 0.08f, 0.31f);
    GLCompat::drawBox(0.10f, 0.71f, 0.31f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.95f, 0.26f);
    GLCompat::drawBox(0.10f, 0.14f, 0.52f);
    GLCompat::drawBox(0.05f, 0.05f, 0.70f);
    glPopMatrix();

    GLCompat::setMaterial(COLOR_GUARD_MAGENTA, 20.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.55f, 0.0f);
    GLCompat::drawSphere(0.24f, 12, 12);
    glPopMatrix();

    GLCompat::setMaterial(COLOR_GUARD_BLACK, 45.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.54f, 0.14f);
    GLCompat::drawSphere(0.17f, 12, 12);

    GLCompat::setEmissive(1.0f, 1.0f, 1.0f, 1.0f);
    glTranslatef(0.0f, 0.02f, 0.14f);
    if (maskShape == 'C') {
        GLCompat::drawTorus(0.015f, 0.065f, 10, 16);
    } else {
        const float symbolSize = (maskShape == 'T') ? 0.10f : 0.12f;
        GLCompat::drawBox(symbolSize, symbolSize, 0.02f);
    }
    GLCompat::resetEmission();
    glPopMatrix();
    glPopMatrix();
}

void GlassBridgeManager::drawContestants(const std::vector<GBContestant>& contestants) {
    for (const GBContestant& contestant : contestants) {
        if (contestant.y <= -38.0f) continue;

        const float fallAngle = contestant.isAlive
            ? 0.0f
            : std::min(contestant.fallVelocity * 12.0f, 90.0f);

        ContestantsManager::drawCharacter(
            contestant.x, contestant.y, contestant.z, 0.0f,
            contestant.legAngle, contestant.armAngle, contestant.bibNumber,
            contestant.isAlive, fallAngle);
    }
}
