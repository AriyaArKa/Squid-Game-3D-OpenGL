#include "environment/Environment.h"
#include "render/Renderer.h"
#include <GL/glut.h>

// The giant overhead 3D scoreboard clock ("05:00") mounted above the finish line. Kept in
// its own file since it's a self-contained widget (digit model + assembly) unrelated to the
// arena shell or the village houses.

namespace {
    // Helper to draw a digital 7-segment character in 3D
    void draw7SegmentDigit3D(int digit, float size, bool flipX = false) {
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
        float rightX = flipX ? -w * 0.5f : w * 0.5f;
        float leftX  = flipX ?  w * 0.5f : -w * 0.5f;

        glPushMatrix();

        // 0: Top
        if (segMap[digit][0]) {
            glPushMatrix(); glTranslatef(0.0f, h, 0.0f); Renderer::drawBox(w, th, 0.02f); glPopMatrix();
        }
        // 1: Top-Right
        if (segMap[digit][1]) {
            glPushMatrix(); glTranslatef(rightX, h * 0.5f, 0.0f); Renderer::drawBox(th, h, 0.02f); glPopMatrix();
        }
        // 2: Bottom-Right
        if (segMap[digit][2]) {
            glPushMatrix(); glTranslatef(rightX, -h * 0.5f, 0.0f); Renderer::drawBox(th, h, 0.02f); glPopMatrix();
        }
        // 3: Bottom
        if (segMap[digit][3]) {
            glPushMatrix(); glTranslatef(0.0f, -h, 0.0f); Renderer::drawBox(w, th, 0.02f); glPopMatrix();
        }
        // 4: Bottom-Left
        if (segMap[digit][4]) {
            glPushMatrix(); glTranslatef(leftX, -h * 0.5f, 0.0f); Renderer::drawBox(th, h, 0.02f); glPopMatrix();
        }
        // 5: Top-Left
        if (segMap[digit][5]) {
            glPushMatrix(); glTranslatef(leftX, h * 0.5f, 0.0f); Renderer::drawBox(th, h, 0.02f); glPopMatrix();
        }
        // 6: Middle
        if (segMap[digit][6]) {
            glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f); Renderer::drawBox(w, th, 0.02f); glPopMatrix();
        }

        glPopMatrix();
    }
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

    // Helper lambda to draw clock face elements at a given Z position
    auto drawClockFace = [&](float zPos) {
        bool frontFacing = (zPos < 0.0f);

        // Cyan Glowing Scoreboard Border
        Renderer::setMaterial(0.15f, 0.85f, 0.95f, 80.0f);
        Renderer::setEmissive(0.1f, 0.9f, 1.0f, 0.8f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, zPos);
        glPushMatrix(); glTranslatef(0.0f, 0.95f, 0.0f); Renderer::drawBox(5.0f, 0.08f, 0.06f); glPopMatrix();
        glPushMatrix(); glTranslatef(0.0f, -0.95f, 0.0f); Renderer::drawBox(5.0f, 0.08f, 0.06f); glPopMatrix();
        glPushMatrix(); glTranslatef(-2.45f, 0.0f, 0.0f); Renderer::drawBox(0.08f, 1.9f, 0.06f); glPopMatrix();
        glPushMatrix(); glTranslatef(2.45f, 0.0f, 0.0f); Renderer::drawBox(0.08f, 1.9f, 0.06f); glPopMatrix();
        glPopMatrix();
        Renderer::resetEmission();

        // Black Inner Display Face
        Renderer::setMaterial(0.04f, 0.04f, 0.05f, 20.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, zPos);
        Renderer::drawBox(4.8f, 1.8f, 0.04f);
        glPopMatrix();

        // Glowing Red Digital 7-Segment Digits: "05:00"
        Renderer::setMaterial(1.0f, 0.15f, 0.15f, 100.0f);
        Renderer::setEmissive(1.0f, 0.08f, 0.08f, 1.0f);

        float digitZ = frontFacing ? (zPos - 0.04f) : (zPos + 0.04f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, digitZ);

        float digitSize = 1.35f;
        // On front face (viewed from z = -52 looking toward +z), world +X is on the viewer's LEFT.
        // So d1 (tens of mins) must be at +1.65f to appear on the left, d4 at -1.65f on the right.
        float posX1 = frontFacing ?  1.65f : -1.65f;
        float posX2 = frontFacing ?  0.60f : -0.60f;
        float posX3 = frontFacing ? -0.60f :  0.60f;
        float posX4 = frontFacing ? -1.65f :  1.65f;

        glPushMatrix(); glTranslatef(posX1, 0.0f, 0.0f); draw7SegmentDigit3D(d1, digitSize, frontFacing); glPopMatrix();
        glPushMatrix(); glTranslatef(posX2, 0.0f, 0.0f); draw7SegmentDigit3D(d2, digitSize, frontFacing); glPopMatrix();

        // Colon dots ":"
        glPushMatrix();
        glTranslatef(0.0f, 0.25f, 0.0f); Renderer::drawBox(0.12f, 0.12f, 0.02f);
        glTranslatef(0.0f, -0.50f, 0.0f); Renderer::drawBox(0.12f, 0.12f, 0.02f);
        glPopMatrix();

        glPushMatrix(); glTranslatef(posX3, 0.0f, 0.0f); draw7SegmentDigit3D(d3, digitSize, frontFacing); glPopMatrix();
        glPushMatrix(); glTranslatef(posX4, 0.0f, 0.0f); draw7SegmentDigit3D(d4, digitSize, frontFacing); glPopMatrix();

        glPopMatrix();
        Renderer::resetEmission();
    };

    // Front side (facing the players and start line, z < 0)
    drawClockFace(-0.29f);
    // Back side (z > 0)
    drawClockFace(0.29f);

    glPopMatrix();
}
