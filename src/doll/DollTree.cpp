#include "doll/DollTree.h"
#include "render/Renderer.h"
#include <GL/glut.h>
#include <cmath>

namespace {
    const float PI = 3.14159265358979323846f;

    // Draw a perfectly connected branch segment with a joint knuckle at the tip
    void drawBranchSegment(float baseR, float tipR, float len) {
        // gluCylinder draws from Z=0 to Z=len
        Renderer::drawCylinder(baseR, tipR, len, 14);

        // Spherical knuckle joint at the tip to ensure 100% seamless branching without gaps
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, len);
        Renderer::drawSphere(tipR * 1.05f, 12, 12);
        glPopMatrix();
    }

    // Recursive / procedural crooked bare twig
    void drawTwig(float baseR, float len, int depth, unsigned int seed) {
        float tipR = baseR * 0.45f;
        drawBranchSegment(baseR, tipR, len);

        if (depth <= 0) return;

        glTranslatef(0.0f, 0.0f, len);

        // Fork into 2 small bare twigs with sharp angular bends (characteristic of dead hardwood)
        float angles[2][2] = {
            { 28.0f + (float)(seed % 15),   35.0f + (float)((seed * 7) % 30) },
            {-26.0f - (float)((seed * 3) % 15), -40.0f - (float)((seed * 11) % 30) }
        };

        for (int i = 0; i < 2; ++i) {
            glPushMatrix();
            glRotatef(angles[i][0], 0.0f, 1.0f, 0.0f);
            glRotatef(angles[i][1], 1.0f, 0.0f, 0.0f);
            drawTwig(tipR, len * 0.70f, depth - 1, seed * 13 + i * 7);
            glPopMatrix();
        }
    }

    // Secondary crooked bough
    void drawSubBough(float baseR, float len, float yaw, float pitch, unsigned int seed) {
        glPushMatrix();
        glRotatef(yaw, 0.0f, 0.0f, 1.0f);
        glRotatef(pitch, 1.0f, 0.0f, 0.0f);

        float midR = baseR * 0.70f;
        float seg1Len = len * 0.55f;
        drawBranchSegment(baseR, midR, seg1Len);

        // Angular bend in mid-branch
        glTranslatef(0.0f, 0.0f, seg1Len);
        glRotatef(15.0f + (float)(seed % 20), 0.0f, 1.0f, 0.0f);
        glRotatef(-10.0f - (float)((seed * 3) % 15), 1.0f, 0.0f, 0.0f);

        float tipR = midR * 0.60f;
        float seg2Len = len * 0.45f;
        drawBranchSegment(midR, tipR, seg2Len);

        // Branch into 2-3 bare twigs
        glTranslatef(0.0f, 0.0f, seg2Len);

        float twigAngles[3][2] = {
            { 32.0f,  25.0f },
            {-30.0f, -20.0f },
            {  5.0f,  38.0f }
        };

        for (int t = 0; t < 3; ++t) {
            glPushMatrix();
            glRotatef(twigAngles[t][0], 0.0f, 1.0f, 0.0f);
            glRotatef(twigAngles[t][1], 1.0f, 0.0f, 0.0f);
            drawTwig(tipR * 0.65f, len * 0.45f, 2, seed + t * 31);
            glPopMatrix();
        }

        glPopMatrix();
    }

    // Sprawling Root Buttress planted in sand
    void drawRootButtress(float angleRad, float length) {
        float rx = cosf(angleRad);
        float rz = sinf(angleRad);

        glPushMatrix();
        glTranslatef(rx * 1.3f, 0.25f, rz * 1.3f);
        glRotatef(angleRad * 180.0f / PI, 0.0f, 1.0f, 0.0f);
        glRotatef(75.0f, 1.0f, 0.0f, 0.0f); // Slanted downward into the sand

        drawBranchSegment(0.65f, 0.38f, length * 0.50f);
        glTranslatef(0.0f, 0.0f, length * 0.50f);
        glRotatef(18.0f, 1.0f, 0.0f, 0.0f); // Flatten along ground
        drawBranchSegment(0.38f, 0.12f, length * 0.50f);

        glPopMatrix();
    }
}

void DollTree::drawDeadTree(float x, float y, float z) {
    // =========================================================================
    // REAL CONTINUOUS 3D GNARLED DEAD TREE (Zero Gaps, 360° Canopy Branching)
    // =========================================================================
    glPushMatrix();
    glTranslatef(x, y, z);

    // Deep weathered hardwood bark color (rich fibrous organic tones)
    const float barkR = 0.24f, barkG = 0.20f, barkB = 0.16f;
    Renderer::setMaterial(barkR, barkG, barkB, 15.0f);

    // 1. Sprawling Root Buttresses radiating into the sand in 360 degrees
    for (int i = 0; i < 8; ++i) {
        float ang = i * (2.0f * PI / 8.0f);
        drawRootButtress(ang, 3.2f);
    }

    // 2. Continuous Lower Trunk (glRotatef -90 on X so Z points straight UP!)
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // Trunk Segment 1: Base to 2.2m
    drawBranchSegment(1.45f, 1.25f, 2.2f);

    // Trunk Segment 2: 2.2m to 4.2m (curving slightly)
    glTranslatef(0.0f, 0.0f, 2.2f);
    glRotatef(3.5f, 1.0f, 0.0f, 0.0f);
    glRotatef(-4.0f, 0.0f, 1.0f, 0.0f);
    drawBranchSegment(1.25f, 1.05f, 2.0f);

    // Trunk Segment 3: 4.2m to 6.0m (tapering to the Grand Fork)
    glTranslatef(0.0f, 0.0f, 2.0f);
    glRotatef(-4.5f, 1.0f, 0.0f, 0.0f);
    glRotatef(5.0f, 0.0f, 1.0f, 0.0f);
    drawBranchSegment(1.05f, 0.88f, 1.8f);

    // AT HEIGHT 6.0m: THE GRAND FORK
    // Huge continuous wooden knuckle sphere
    glTranslatef(0.0f, 0.0f, 1.8f);
    Renderer::drawSphere(0.92f, 16, 16);

    // =========================================================================
    // 3. FIVE MAJOR GNARLED BOUGHS SPREADING ACROSS ALL 360 DEGREES!
    // =========================================================================

    // BOUGH 1: Major Left Bough (Spreading Left and Forward)
    drawSubBough(0.56f, 3.6f, -48.0f, 38.0f, 101);

    // BOUGH 2: Major Right Bough (Spreading Right and Forward)
    drawSubBough(0.56f, 3.6f,  48.0f, 38.0f, 202);

    // BOUGH 3: Major Rear-Left Bough (Spreading Left and Back toward Wall)
    drawSubBough(0.52f, 3.4f, -135.0f, 32.0f, 303);

    // BOUGH 4: Major Rear-Right Bough (Spreading Right and Back toward Wall)
    drawSubBough(0.52f, 3.4f,  135.0f, 32.0f, 404);

    // BOUGH 5: Forward-Upper Canopy Bough (Spreading directly over Doll's head)
    drawSubBough(0.50f, 3.2f, 180.0f, 28.0f, 505);

    // BOUGH 6: Center-Top Towering Spire (Reaches 14-15 meters tall!)
    glPushMatrix();
    glRotatef(8.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(12.0f, 0.0f, 1.0f, 0.0f);
    drawBranchSegment(0.65f, 0.42f, 3.5f);

    glTranslatef(0.0f, 0.0f, 3.5f);
    // Spire crown forks into 3 high twigs
    float spireAngles[3][2] = {
        { 18.0f,  22.0f },
        {-20.0f, -18.0f },
        { 32.0f, -24.0f }
    };
    for (int s = 0; s < 3; ++s) {
        glPushMatrix();
        glRotatef(spireAngles[s][0], 0.0f, 1.0f, 0.0f);
        glRotatef(spireAngles[s][1], 1.0f, 0.0f, 0.0f);
        drawTwig(0.24f, 2.8f, 2, 606 + s * 17);
        glPopMatrix();
    }
    glPopMatrix();

    glPopMatrix(); // End Trunk

    glPopMatrix(); // End Tree
}
