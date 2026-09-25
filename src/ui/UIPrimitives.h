#ifndef UI_PRIMITIVES_H
#define UI_PRIMITIVES_H

#include <GL/glut.h>
#include <cmath>

// Low-level 2D drawing toolkit shared by the HUD (text, filled/bordered rects, circles, the
// Squid Game ○△□ logo, a 7-segment digit, and the assembled digital timer widget). UI.cpp
// composes these into the actual sidebar/overlay panels; this header only knows how to draw
// primitives in an already-set-up 2D ortho projection.
namespace UIPrimitives {
    inline void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
        glRasterPos2f(x, y);
        for (const char* c = text; *c != '\0'; ++c) {
            glutBitmapCharacter(font, *c);
        }
    }

    inline void drawFilledRect(float x, float y, float w, float h, float r, float g, float b, float a = 1.0f) {
        glColor4f(r, g, b, a);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }

    inline void drawBorderRect(float x, float y, float w, float h, float lineWidth, float r, float g, float b, float a = 1.0f) {
        glColor4f(r, g, b, a);
        glLineWidth(lineWidth);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }

    inline void drawCircle(float cx, float cy, float radius, int segments = 24) {
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; ++i) {
            float theta = 2.0f * 3.14159265f * (float)i / (float)segments;
            glVertex2f(cx + cosf(theta) * radius, cy + sinf(theta) * radius);
        }
        glEnd();
    }

    inline void drawSquidGameSymbols(float cx, float cy, float size) {
        glLineWidth(3.0f);
        glColor3f(0.95f, 0.20f, 0.55f); // Hot pink

        // Circle
        drawCircle(cx - size * 1.8f, cy, size * 0.7f);

        // Triangle
        glBegin(GL_LINE_LOOP);
        glVertex2f(cx, cy + size * 0.8f);
        glVertex2f(cx - size * 0.7f, cy - size * 0.6f);
        glVertex2f(cx + size * 0.7f, cy - size * 0.6f);
        glEnd();

        // Square
        glBegin(GL_LINE_LOOP);
        glVertex2f(cx + size * 1.8f - size * 0.65f, cy - size * 0.65f);
        glVertex2f(cx + size * 1.8f + size * 0.65f, cy - size * 0.65f);
        glVertex2f(cx + size * 1.8f + size * 0.65f, cy + size * 0.65f);
        glVertex2f(cx + size * 1.8f - size * 0.65f, cy + size * 0.65f);
        glEnd();
    }

    inline void draw2D7SegmentDigit(float x, float y, int digit, float size) {
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

        if (digit < 0 || digit > 9) return;

        float w = size * 0.5f;
        float h = size * 0.5f;
        float t = size * 0.10f;

        // 0: Top
        if (segs[digit][0]) drawFilledRect(x - w*0.5f, y + h - t*0.5f, w, t, 1.0f, 0.1f, 0.1f);
        // 1: Top-Right
        if (segs[digit][1]) drawFilledRect(x + w*0.5f - t*0.5f, y, t, h, 1.0f, 0.1f, 0.1f);
        // 2: Bot-Right
        if (segs[digit][2]) drawFilledRect(x + w*0.5f - t*0.5f, y - h, t, h, 1.0f, 0.1f, 0.1f);
        // 3: Bottom
        if (segs[digit][3]) drawFilledRect(x - w*0.5f, y - h - t*0.5f, w, t, 1.0f, 0.1f, 0.1f);
        // 4: Bot-Left
        if (segs[digit][4]) drawFilledRect(x - w*0.5f - t*0.5f, y - h, t, h, 1.0f, 0.1f, 0.1f);
        // 5: Top-Left
        if (segs[digit][5]) drawFilledRect(x - w*0.5f - t*0.5f, y, t, h, 1.0f, 0.1f, 0.1f);
        // 6: Middle
        if (segs[digit][6]) drawFilledRect(x - w*0.5f, y - t*0.5f, w, t, 1.0f, 0.1f, 0.1f);
    }

    inline void drawHUDDigitalTimer(float cx, float cy, float remainingSeconds) {
        if (remainingSeconds < 0.0f) remainingSeconds = 0.0f;
        int totalSec = (int)remainingSeconds;
        int mins = totalSec / 60;
        int secs = totalSec % 60;

        int d1 = mins / 10;
        int d2 = mins % 10;
        int d3 = secs / 10;
        int d4 = secs % 10;

        float boxW = 168.0f;
        float boxH = 54.0f;
        float x = cx - boxW * 0.5f;
        float y = cy - boxH * 0.5f;

        // Outer Dark Beveled Box
        drawFilledRect(x, y, boxW, boxH, 0.08f, 0.08f, 0.10f, 0.90f);
        // Cyan Neon Border
        drawBorderRect(x + 3.0f, y + 3.0f, boxW - 6.0f, boxH - 6.0f, 2.5f, 0.15f, 0.85f, 0.95f, 0.95f);

        // Digits
        float digitSize = 20.0f;
        draw2D7SegmentDigit(cx - 46.0f, cy, d1, digitSize);
        draw2D7SegmentDigit(cx - 18.0f, cy, d2, digitSize);

        // Colon dots
        drawFilledRect(cx - 4.0f, cy + 5.0f, 4.0f, 4.0f, 1.0f, 0.1f, 0.1f);
        drawFilledRect(cx - 4.0f, cy - 7.0f, 4.0f, 4.0f, 1.0f, 0.1f, 0.1f);

        draw2D7SegmentDigit(cx + 14.0f, cy, d3, digitSize);
        draw2D7SegmentDigit(cx + 42.0f, cy, d4, digitSize);
    }

    // Sets up an orthographic 2D projection matching the given pixel dimensions, disables
    // depth/lighting, and enables alpha blending. Pair every begin2D() with an end2D().
    inline void begin2D(int windowWidth, int windowHeight) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0.0, windowWidth, 0.0, windowHeight, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    inline void end2D() {
        glPopAttrib();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}

#endif // UI_PRIMITIVES_H
