#include "guards/Guards.h"
#include "render/Renderer.h"
#include "audio/Audio.h"
#include "core/Config.h"
#include <GL/glut.h>
#include "engine/GLCompat.h"
#include <cmath>

namespace {
    void drawMaskSymbol(GuardType type) {
        Renderer::setMaterial(0.98f, 0.98f, 0.98f, 20.0f);
        Renderer::setEmissive(0.95f, 0.95f, 0.95f, 0.6f);

        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.285f);

        if (type == GUARD_CIRCLE) {
            // White circle ring with thickness
            Renderer::drawTorus(0.016f, 0.088f, 12, 24);
        } else if (type == GUARD_TRIANGLE) {
            // White equilateral triangle composed of 3 solid segments
            // Base
            glPushMatrix();
            glTranslatef(0.0f, -0.065f, 0.0f);
            glScalef(0.16f, 0.018f, 0.018f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Left side (leans right so the two sides meet at the top: an upright triangle)
            glPushMatrix();
            glTranslatef(-0.042f, 0.012f, 0.0f);
            glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
            glScalef(0.018f, 0.165f, 0.018f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Right side
            glPushMatrix();
            glTranslatef(0.042f, 0.012f, 0.0f);
            glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
            glScalef(0.018f, 0.165f, 0.018f);
            glutSolidCube(1.0);
            glPopMatrix();
        } else if (type == GUARD_SQUARE) {
            // White square composed of 4 solid segments
            // Top
            glPushMatrix();
            glTranslatef(0.0f, 0.075f, 0.0f);
            glScalef(0.168f, 0.018f, 0.018f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Bottom
            glPushMatrix();
            glTranslatef(0.0f, -0.075f, 0.0f);
            glScalef(0.168f, 0.018f, 0.018f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Left
            glPushMatrix();
            glTranslatef(-0.075f, 0.0f, 0.0f);
            glScalef(0.018f, 0.168f, 0.018f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Right
            glPushMatrix();
            glTranslatef(0.075f, 0.0f, 0.0f);
            glScalef(0.018f, 0.168f, 0.018f);
            glutSolidCube(1.0);
            glPopMatrix();
        }

        glPopMatrix();
        Renderer::resetEmission();
    }

    void drawGun(bool isShooting) {
        // MP5 / Submachine gun held across chest
        Renderer::setMaterial(0.12f, 0.12f, 0.14f, 40.0f);

        glPushMatrix();
        glTranslatef(0.12f, 0.85f, 0.35f);
        glRotatef(-15.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(5.0f, 1.0f, 0.0f, 0.0f);

        // Receiver / body
        Renderer::drawBox(0.08f, 0.12f, 0.55f);

        // Barrel
        glPushMatrix();
        glTranslatef(0.0f, 0.02f, 0.32f);
        Renderer::drawBox(0.04f, 0.04f, 0.22f);
        glPopMatrix();

        // Magazine (curved clip)
        glPushMatrix();
        glTranslatef(0.0f, -0.12f, 0.08f);
        glRotatef(12.0f, 1.0f, 0.0f, 0.0f);
        Renderer::drawBox(0.04f, 0.20f, 0.08f);
        glPopMatrix();

        // Muzzle Flash
        if (isShooting) {
            glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
            GLCompat::setUnlit(true);
            glColor3f(1.0f, 0.85f, 0.2f);
            glPushMatrix();
            glTranslatef(0.0f, 0.02f, 0.48f);
            Renderer::drawSphere(0.14f, 8, 8);
            glPopMatrix();
            glPopAttrib();
        }

        glPopMatrix();
    }
}

void GuardsManager::init(std::vector<Guard>& guards) {
    guards.clear();

    // 2 Guards flanking the doll at the finish line (Triangle soldiers)
    Guard leftGuard;
    leftGuard.x = -6.0f;
    leftGuard.y = 0.0f;
    leftGuard.z = 44.0f;
    leftGuard.facingAngle = 180.0f; // facing arena
    leftGuard.type = GUARD_TRIANGLE;
    leftGuard.isShooting = false;
    leftGuard.shootTimer = 0.0f;
    guards.push_back(leftGuard);

    Guard rightGuard;
    rightGuard.x = 6.0f;
    rightGuard.y = 0.0f;
    rightGuard.z = 44.0f;
    rightGuard.facingAngle = 180.0f;
    rightGuard.type = GUARD_TRIANGLE;
    rightGuard.isShooting = false;
    rightGuard.shootTimer = 0.0f;
    guards.push_back(rightGuard);

    // 1 Guard Manager (Square) at the finish line area
    Guard captain;
    captain.x = 8.5f;
    captain.y = 0.0f;
    captain.z = 48.0f;
    captain.facingAngle = 180.0f;
    captain.type = GUARD_SQUARE;
    captain.isShooting = false;
    captain.shootTimer = 0.0f;
    guards.push_back(captain);

    // Perimeter Worker Guards (Circle) along sidewalls
    Guard side1;
    side1.x = -18.5f; side1.y = 0.0f; side1.z = 0.0f;
    side1.facingAngle = 90.0f;
    side1.type = GUARD_CIRCLE;
    side1.isShooting = false;
    side1.shootTimer = 0.0f;
    guards.push_back(side1);

    Guard side2;
    side2.x = 18.5f; side2.y = 0.0f; side2.z = 0.0f;
    side2.facingAngle = -90.0f;
    side2.type = GUARD_CIRCLE;
    side2.isShooting = false;
    side2.shootTimer = 0.0f;
    guards.push_back(side2);
}

void GuardsManager::update(std::vector<Guard>& guards, float dt) {
    for (auto& g : guards) {
        if (g.isShooting) {
            g.shootTimer -= dt;
            if (g.shootTimer <= 0.0f) {
                g.isShooting = false;
            }
        }
    }
}

void GuardsManager::triggerShot(std::vector<Guard>& guards, float targetX, float targetZ) {
    // Find closest guard to aim and shoot
    int bestIdx = 0;
    float bestDistSq = 999999.0f;
    for (size_t i = 0; i < guards.size(); ++i) {
        float dx = guards[i].x - targetX;
        float dz = guards[i].z - targetZ;
        float dsq = dx*dx + dz*dz;
        if (dsq < bestDistSq) {
            bestDistSq = dsq;
            bestIdx = (int)i;
        }
    }
    guards[bestIdx].isShooting = true;
    guards[bestIdx].shootTimer = 0.35f;

    // Face the target
    float dx = targetX - guards[bestIdx].x;
    float dz = targetZ - guards[bestIdx].z;
    guards[bestIdx].facingAngle = atan2f(dx, dz) * 180.0f / 3.14159265f;

    AudioManager::playGunshot();
}

void GuardsManager::drawSingleGuard(const Guard& guard) {
    glPushMatrix();
    glTranslatef(guard.x, guard.y, guard.z);
    glRotatef(guard.facingAngle, 0.0f, 1.0f, 0.0f);
    glScalef(1.2f, 1.2f, 1.2f);

    // Ground shadow
    Renderer::drawGroundShadow(0.0f, 0.0f, 0.45f, 0.40f, 0.35f);

    // Authentic Squid Game hot magenta jumpsuit tone from central Config.h
    const float pr = COLOR_GUARD_MAGENTA.r, pg = COLOR_GUARD_MAGENTA.g, pb = COLOR_GUARD_MAGENTA.b;

    // Boots (Black combat boots with rubber sole)
    Renderer::setMaterial(0.10f, 0.10f, 0.12f, 30.0f);
    glPushMatrix();
    glTranslatef(-0.16f, 0.10f, 0.03f);
    glScalef(0.18f, 0.20f, 0.28f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.16f, 0.10f, 0.03f);
    glScalef(0.18f, 0.20f, 0.28f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Legs (Magenta Jumpsuit pants)
    Renderer::setMaterial(pr, pg, pb, 20.0f);
    glPushMatrix();
    glTranslatef(-0.16f, 0.42f, 0.0f);
    glScalef(0.20f, 0.50f, 0.22f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.16f, 0.42f, 0.0f);
    glScalef(0.20f, 0.50f, 0.22f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Torso (Magenta Jumpsuit)
    glPushMatrix();
    glTranslatef(0.0f, 0.95f, 0.0f);
    glScalef(0.55f, 0.60f, 0.32f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Black Central Front Zipper
    Renderer::setMaterial(0.08f, 0.08f, 0.09f, 35.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.95f, 0.165f);
    glScalef(0.024f, 0.58f, 0.015f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Black Belt
    Renderer::setMaterial(0.08f, 0.08f, 0.09f, 40.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.70f, 0.0f);
    glScalef(0.57f, 0.12f, 0.34f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Silver Metallic Belt Buckle
    Renderer::setMaterial(0.85f, 0.85f, 0.88f, 70.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.70f, 0.175f);
    glScalef(0.12f, 0.09f, 0.025f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Black Tactical Shoulder Straps (Harness)
    Renderer::setMaterial(0.08f, 0.08f, 0.09f, 35.0f);
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(side * 0.14f, 0.98f, 0.163f);
        glScalef(0.045f, 0.50f, 0.015f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    // Hood Neck Cowl / Collar
    Renderer::setMaterial(pr, pg, pb, 15.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.22f, 0.0f);
    glScalef(0.34f, 0.09f, 0.32f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Arms & Black Gloves
    // Left Arm
    Renderer::setMaterial(pr, pg, pb, 20.0f);
    glPushMatrix();
    glTranslatef(-0.35f, 0.95f, 0.08f);
    glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
    glScalef(0.16f, 0.45f, 0.16f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Right Arm
    glPushMatrix();
    glTranslatef(0.35f, 0.95f, 0.10f);
    glRotatef(35.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(-25.0f, 0.0f, 1.0f, 0.0f);
    glScalef(0.16f, 0.45f, 0.16f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Black Gloves
    Renderer::setMaterial(0.08f, 0.08f, 0.09f, 30.0f);
    glPushMatrix();
    glTranslatef(-0.25f, 0.80f, 0.22f);
    Renderer::drawSphere(0.09f, 8, 8);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.20f, 0.80f, 0.25f);
    Renderer::drawSphere(0.09f, 8, 8);
    glPopMatrix();

    // Hood (Pink)
    Renderer::setMaterial(pr, pg, pb, 15.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.35f, -0.04f);
    glScalef(0.36f, 0.38f, 0.36f);
    glutSolidSphere(1.0, 16, 16);
    glPopMatrix();

    // Mask (Black Mesh face with shape symbol)
    Renderer::setMaterial(0.05f, 0.05f, 0.06f, 50.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.35f, 0.06f);
    glScalef(0.30f, 0.32f, 0.28f);
    glutSolidSphere(1.0, 16, 16);
    glPopMatrix();

    // Mask shape symbol (Circle, Triangle, Square)
    glPushMatrix();
    glTranslatef(0.0f, 1.35f, 0.06f);
    drawMaskSymbol(guard.type);
    glPopMatrix();

    // Weapon
    drawGun(guard.isShooting);

    glPopMatrix();
}

void GuardsManager::draw(const std::vector<Guard>& guards) {
    for (const auto& g : guards) {
        drawSingleGuard(g);
    }
}
