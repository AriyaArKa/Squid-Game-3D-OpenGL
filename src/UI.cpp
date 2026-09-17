#include "UI.h"
#include "Config.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace {
    void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
        glRasterPos2f(x, y);
        for (const char* c = text; *c != '\0'; ++c) {
            glutBitmapCharacter(font, *c);
        }
    }

    void drawFilledRect(float x, float y, float w, float h, float r, float g, float b, float a = 1.0f) {
        glColor4f(r, g, b, a);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }

    void drawBorderRect(float x, float y, float w, float h, float lineWidth, float r, float g, float b, float a = 1.0f) {
        glColor4f(r, g, b, a);
        glLineWidth(lineWidth);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }

    void drawCircle(float cx, float cy, float radius, int segments = 24) {
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; ++i) {
            float theta = 2.0f * 3.14159265f * (float)i / (float)segments;
            glVertex2f(cx + cosf(theta) * radius, cy + sinf(theta) * radius);
        }
        glEnd();
    }

    void drawSquidGameSymbols(float cx, float cy, float size) {
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

    void draw2D7SegmentDigit(float x, float y, int digit, float size) {
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

    void drawHUDDigitalTimer(float cx, float cy, float remainingSeconds) {
        if (remainingSeconds < 0.0f) remainingSeconds = 0.0f;
        int totalSec = (int)remainingSeconds;
        int mins = totalSec / 60;
        int secs = totalSec % 60;

        int d1 = mins / 10;
        int d2 = mins % 10;
        int d3 = secs / 10;
        int d4 = secs % 10;

        float boxW = 180.0f;
        float boxH = 65.0f;
        float x = cx - boxW * 0.5f;
        float y = cy - boxH * 0.5f;

        // Outer Dark Beveled Box
        drawFilledRect(x, y, boxW, boxH, 0.08f, 0.08f, 0.10f, 0.90f);
        // Cyan Neon Border
        drawBorderRect(x + 3.0f, y + 3.0f, boxW - 6.0f, boxH - 6.0f, 2.5f, 0.15f, 0.85f, 0.95f, 0.95f);

        // Digits
        float digitSize = 24.0f;
        draw2D7SegmentDigit(cx - 52.0f, cy, d1, digitSize);
        draw2D7SegmentDigit(cx - 20.0f, cy, d2, digitSize);

        // Colon dots
        drawFilledRect(cx - 4.0f, cy + 6.0f, 4.0f, 4.0f, 1.0f, 0.1f, 0.1f);
        drawFilledRect(cx - 4.0f, cy - 8.0f, 4.0f, 4.0f, 1.0f, 0.1f, 0.1f);

        draw2D7SegmentDigit(cx + 16.0f, cy, d3, digitSize);
        draw2D7SegmentDigit(cx + 48.0f, cy, d4, digitSize);
    }
}

void UIManager::init() {
}

void UIManager::renderHUD(int windowWidth, int windowHeight,
                          GameState gameState, LightState lightState,
                          float gameTimer, int aliveCount,
                          const Player& player, CameraMode camMode) {
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

    // 1. Top Bar: Squid Game Logo, Digital Countdown Timer, Live Player Counter
    float topY = windowHeight - 45.0f;

    // Squid Game Logo (○ △ □) on Top Left
    drawSquidGameSymbols(85.0f, topY, 15.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(150.0f, topY - 6.0f, "SQUID GAME : RED LIGHT, GREEN LIGHT", GLUT_BITMAP_HELVETICA_18);

    // Top Center: Digital Clock (05:00)
    drawHUDDigitalTimer(windowWidth * 0.5f, windowHeight - 45.0f, gameTimer);

    // Top Right: Live Alive Counter (ALIVE: 456 / 456)
    float counterBoxW = 200.0f;
    float counterBoxH = 45.0f;
    float counterX = windowWidth - counterBoxW - 20.0f;
    float counterY = windowHeight - counterBoxH - 22.0f;

    drawFilledRect(counterX, counterY, counterBoxW, counterBoxH, 0.08f, 0.08f, 0.10f, 0.85f);
    drawBorderRect(counterX, counterY, counterBoxW, counterBoxH, 2.0f, 0.94f, 0.16f, 0.48f); // Hot pink border

    char aliveBuf[64];
    sprintf(aliveBuf, "PLAYERS: %d / %d", aliveCount, TOTAL_CONTESTANTS);
    glColor3f(1.0f, 0.95f, 0.20f); // Bright yellow text
    drawText(counterX + 16.0f, counterY + 14.0f, aliveBuf, GLUT_BITMAP_HELVETICA_18);

    // 2. Light State Banner (PLAYING state)
    if (gameState == PLAYING) {
        float bannerW = 280.0f;
        float bannerH = 42.0f;
        float bannerX = windowWidth * 0.5f - bannerW * 0.5f;
        float bannerY = windowHeight - 115.0f;

        if (lightState == GREEN_LIGHT) {
            drawFilledRect(bannerX, bannerY, bannerW, bannerH, 0.05f, 0.65f, 0.25f, 0.88f);
            drawBorderRect(bannerX, bannerY, bannerW, bannerH, 2.5f, 0.2f, 1.0f, 0.4f);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawText(bannerX + 35.0f, bannerY + 13.0f, ">> GREEN LIGHT — MOVE! <<", GLUT_BITMAP_HELVETICA_18);
        } else {
            drawFilledRect(bannerX, bannerY, bannerW, bannerH, 0.85f, 0.08f, 0.08f, 0.92f);
            drawBorderRect(bannerX, bannerY, bannerW, bannerH, 2.5f, 1.0f, 0.3f, 0.3f);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawText(bannerX + 30.0f, bannerY + 13.0f, "!! RED LIGHT — FREEZE !!", GLUT_BITMAP_HELVETICA_18);
        }

        // Distance progress bar to finish line
        float totalDist = FINISH_Z - START_Z;
        float currDist = player.z - START_Z;
        float progress = currDist / totalDist;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;

        float pbarW = 340.0f;
        float pbarH = 14.0f;
        float pbarX = windowWidth * 0.5f - pbarW * 0.5f;
        float pbarY = 55.0f;

        drawFilledRect(pbarX, pbarY, pbarW, pbarH, 0.15f, 0.15f, 0.18f, 0.8f);
        drawFilledRect(pbarX, pbarY, pbarW * progress, pbarH, 0.94f, 0.16f, 0.48f, 0.95f);
        drawBorderRect(pbarX, pbarY, pbarW, pbarH, 1.5f, 1.0f, 1.0f, 1.0f, 0.8f);

        char distBuf[64];
        sprintf(distBuf, "COURSE PROGRESS: %d%% TO FINISH LINE", (int)(progress * 100.0f));
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(pbarX + 40.0f, pbarY - 18.0f, distBuf, GLUT_BITMAP_HELVETICA_12);

        // Dynamic Speed Badge on Bottom Right of screen
        float speedBoxW = 210.0f;
        float speedBoxH = 34.0f;
        float speedBoxX = windowWidth - speedBoxW - 20.0f;
        float speedBoxY = 46.0f;

        drawFilledRect(speedBoxX, speedBoxY, speedBoxW, speedBoxH, 0.08f, 0.08f, 0.12f, 0.85f);
        if (player.speedTier == SPEED_WALK) {
            drawBorderRect(speedBoxX, speedBoxY, speedBoxW, speedBoxH, 2.0f, 0.2f, 0.85f, 1.0f);
            glColor3f(0.3f, 0.9f, 1.0f);
            drawText(speedBoxX + 14.0f, speedBoxY + 10.0f, "PACE: [SLOW WALK] (Z)", GLUT_BITMAP_HELVETICA_12);
        } else if (player.speedTier == SPEED_SPRINT) {
            drawBorderRect(speedBoxX, speedBoxY, speedBoxW, speedBoxH, 2.0f, 1.0f, 0.35f, 0.15f);
            glColor3f(1.0f, 0.45f, 0.2f);
            drawText(speedBoxX + 10.0f, speedBoxY + 10.0f, "PACE: [FAST SPRINT] (Shift/C)", GLUT_BITMAP_HELVETICA_12);
        } else {
            drawBorderRect(speedBoxX, speedBoxY, speedBoxW, speedBoxH, 2.0f, 0.2f, 1.0f, 0.4f);
            glColor3f(0.25f, 1.0f, 0.45f);
            drawText(speedBoxX + 14.0f, speedBoxY + 10.0f, "PACE: [EASY RUN] (X)", GLUT_BITMAP_HELVETICA_12);
        }
    }

    // Bottom Controls Bar
    float botY = 16.0f;
    glColor3f(0.88f, 0.88f, 0.88f);
    char ctrlBuf[256];
    const char* camStr = "3rd Person Follow";
    switch (camMode) {
        case CAM_THIRD_PERSON: camStr = "3rd Person [1]"; break;
        case CAM_FIRST_PERSON: camStr = "1st Person [2]"; break;
        case CAM_TOP_DOWN:     camStr = "Top-Down Aerial Floor [3]"; break;
        case CAM_DOLL_CLOSEUP: camStr = "Doll 360 Orbit & Surroundings [4]"; break;
        case CAM_SIDE_LEFT:    camStr = "Left Stadium [5]"; break;
        case CAM_SIDE_RIGHT:   camStr = "Right Stadium [6]"; break;
        case CAM_DOLL_POV:     camStr = "Doll Scanning POV [7]"; break;
        case CAM_START_LINE:   camStr = "Start Line [8]"; break;
        case CAM_FINISH_LINE:  camStr = "Finish Line [9]"; break;
        case CAM_OVERHEAD:     camStr = "Overhead Panoramic Drone [0]"; break;
    }
    sprintf(ctrlBuf, "Controls: W/S/A/D = Move | Z/X/C = Walk/Run/Sprint | 1-0 = Cameras (%s) | Mouse Drag = 360 Orbit | R = Restart", camStr);
    drawText(20.0f, botY, ctrlBuf, GLUT_BITMAP_HELVETICA_12);

    // 3. Overlays for Game States
    if (gameState == WAITING) {
        // Dark Cinematic Backdrop
        drawFilledRect(0, 0, (float)windowWidth, (float)windowHeight, 0.0f, 0.0f, 0.0f, 0.65f);

        float modalW = 820.0f;
        if (modalW > windowWidth - 40.0f) modalW = windowWidth - 40.0f;
        float modalH = 540.0f;
        if (modalH > windowHeight - 40.0f) modalH = windowHeight - 40.0f;
        float modalX = windowWidth * 0.5f - modalW * 0.5f;
        float modalY = windowHeight * 0.5f - modalH * 0.5f;

        drawFilledRect(modalX, modalY, modalW, modalH, 0.06f, 0.06f, 0.09f, 0.95f);
        drawBorderRect(modalX, modalY, modalW, modalH, 3.5f, 0.94f, 0.16f, 0.48f); // Hot pink border
        drawBorderRect(modalX + 3.0f, modalY + 3.0f, modalW - 6.0f, modalH - 6.0f, 1.0f, 0.15f, 0.85f, 0.95f, 0.5f);

        // Header Title
        drawSquidGameSymbols(windowWidth * 0.5f, modalY + modalH - 38.0f, 18.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(modalX + modalW * 0.5f - 175.0f, modalY + modalH - 72.0f, "GAME 1 : RED LIGHT, GREEN LIGHT", GLUT_BITMAP_TIMES_ROMAN_24);

        // Decorative separator
        drawFilledRect(modalX + 30.0f, modalY + modalH - 85.0f, modalW - 60.0f, 2.0f, 0.94f, 0.16f, 0.48f, 0.8f);

        // 2-COLUMN LAYOUT FOR RULES & CONTROLS
        float col1X = modalX + 35.0f;
        float col2X = modalX + modalW * 0.5f + 10.0f;
        float colY = modalY + modalH - 115.0f;

        // --- LEFT COLUMN: RULES & SPEED CONTROLS ---
        glColor3f(1.0f, 0.85f, 0.20f); // Gold section title
        drawText(col1X, colY, "=== GAME RULES & OBJECTIVE ===", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.92f, 0.92f, 0.92f);
        drawText(col1X, colY - 24.0f, "* GREEN LIGHT: Doll sings. Move forward!", GLUT_BITMAP_HELVETICA_12);
        drawText(col1X, colY - 44.0f, "* RED LIGHT: Doll turns head. FREEZE instantly!", GLUT_BITMAP_HELVETICA_12);
        drawText(col1X, colY - 64.0f, "* Any motion during Red Light = ELIMINATED.", GLUT_BITMAP_HELVETICA_12);
        drawText(col1X, colY - 84.0f, "* Cross pink finish line (97m) before 05:00.", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.2f, 0.9f, 1.0f); // Cyan speed title
        drawText(col1X, colY - 120.0f, "=== PLAYER 456 SPEED SYSTEM ===", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.4f, 0.95f, 1.0f);
        drawText(col1X, colY - 144.0f, "[Z] or [Ctrl/Alt] : SLOW WALK (Safe & precise)", GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.3f, 1.0f, 0.5f);
        drawText(col1X, colY - 164.0f, "[X]               : EASY RUN / JOG (Default pace)", GLUT_BITMAP_HELVETICA_12);
        glColor3f(1.0f, 0.5f, 0.25f);
        drawText(col1X, colY - 184.0f, "[C] or [Shift]    : FAST SPRINT (Maximum speed)", GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.85f, 0.85f, 0.85f);
        drawText(col1X, colY - 204.0f, "[G]               : Cycle Speed Gears (1-3)", GLUT_BITMAP_HELVETICA_12);

        // --- RIGHT COLUMN: MOVEMENT & 360° CAMERAS ---
        glColor3f(1.0f, 0.85f, 0.20f);
        drawText(col2X, colY, "=== NAVIGATION & CONTROLS ===", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.92f, 0.92f, 0.92f);
        drawText(col2X, colY - 24.0f, "W / S or Up / Down  : Move Forward / Backward", GLUT_BITMAP_HELVETICA_12);
        drawText(col2X, colY - 44.0f, "A / D or Left / Right: Turn Player Left / Right", GLUT_BITMAP_HELVETICA_12);
        drawText(col2X, colY - 64.0f, "Q / E               : Strafe Left / Right", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.94f, 0.40f, 0.70f); // Pink camera title
        drawText(col2X, colY - 100.0f, "=== 360 CAMERAS & EXPLORATION ===", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.92f, 0.92f, 0.92f);
        drawText(col2X, colY - 124.0f, "[1] 3rd Person Follow (Player 456 Orbit)", GLUT_BITMAP_HELVETICA_12);
        drawText(col2X, colY - 144.0f, "[2] 1st Person Eyes | [3] Top Aerial Floor", GLUT_BITMAP_HELVETICA_12);
        glColor3f(1.0f, 0.95f, 0.3f);
        drawText(col2X, colY - 164.0f, "[4] DOLL 360 ORBIT & ARENA SURROUNDINGS", GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.92f, 0.92f, 0.92f);
        drawText(col2X, colY - 184.0f, "[5/6] Left/Right Stadium | [7] Doll POV", GLUT_BITMAP_HELVETICA_12);
        drawText(col2X, colY - 204.0f, "[8/9] Start/Finish Lines | [0] Panoramic Drone", GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.2f, 0.9f, 1.0f);
        drawText(col2X, colY - 224.0f, "Mouse Drag: 360 Orbit in any view | Wheel: Zoom", GLUT_BITMAP_HELVETICA_12);

        // Bottom Flashing Start Prompt Box
        float promptBoxW = modalW - 60.0f;
        float promptBoxH = 44.0f;
        float promptBoxX = modalX + 30.0f;
        float promptBoxY = modalY + 22.0f;

        drawFilledRect(promptBoxX, promptBoxY, promptBoxW, promptBoxH, 0.94f, 0.16f, 0.48f, 0.25f);
        drawBorderRect(promptBoxX, promptBoxY, promptBoxW, promptBoxH, 2.0f, 0.94f, 0.16f, 0.48f);

        glColor3f(1.0f, 0.95f, 0.20f); // Pulsing yellow text
        drawText(promptBoxX + promptBoxW * 0.5f - 160.0f, promptBoxY + 14.0f, ">> PRESS SPACE OR ENTER TO START <<", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == CAUGHT) {
        // Red Blood Vignette Overlay
        drawFilledRect(0, 0, (float)windowWidth, (float)windowHeight, 0.5f, 0.0f, 0.0f, 0.45f);

        float modalW = 500.0f;
        float modalH = 260.0f;
        float modalX = windowWidth * 0.5f - modalW * 0.5f;
        float modalY = windowHeight * 0.5f - modalH * 0.5f;

        drawFilledRect(modalX, modalY, modalW, modalH, 0.12f, 0.02f, 0.02f, 0.94f);
        drawBorderRect(modalX, modalY, modalW, modalH, 3.5f, 1.0f, 0.1f, 0.1f);

        glColor3f(1.0f, 0.15f, 0.15f);
        drawText(modalX + 105.0f, modalY + modalH - 65.0f, "PLAYER 456 — ELIMINATED!", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(0.9f, 0.8f, 0.8f);
        drawText(modalX + 115.0f, modalY + modalH - 115.0f, "Movement detected during RED LIGHT.", GLUT_BITMAP_HELVETICA_18);

        glColor3f(1.0f, 0.9f, 0.2f);
        drawText(modalX + 140.0f, modalY + 50.0f, "Press 'R' to Retry Game", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == WON) {
        // Golden Victory Glow
        drawFilledRect(0, 0, (float)windowWidth, (float)windowHeight, 0.1f, 0.35f, 0.2f, 0.35f);

        float modalW = 540.0f;
        float modalH = 280.0f;
        float modalX = windowWidth * 0.5f - modalW * 0.5f;
        float modalY = windowHeight * 0.5f - modalH * 0.5f;

        drawFilledRect(modalX, modalY, modalW, modalH, 0.04f, 0.12f, 0.08f, 0.95f);
        drawBorderRect(modalX, modalY, modalW, modalH, 3.5f, 0.2f, 1.0f, 0.4f);

        glColor3f(0.2f, 1.0f, 0.4f);
        drawText(modalX + 95.0f, modalY + modalH - 60.0f, "CONGRATULATIONS! YOU SURVIVED!", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(1.0f, 0.92f, 0.2f);
        drawText(modalX + 130.0f, modalY + modalH - 115.0f, "PRIZE: 45.6 BILLION WON", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.9f, 0.9f, 0.9f);
        drawText(modalX + 120.0f, modalY + modalH - 155.0f, "You reached the finish line safely.", GLUT_BITMAP_HELVETICA_18);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(modalX + 145.0f, modalY + 45.0f, "Press 'R' to Play Again", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == TIME_UP) {
        drawFilledRect(0, 0, (float)windowWidth, (float)windowHeight, 0.3f, 0.0f, 0.0f, 0.5f);

        float modalW = 480.0f;
        float modalH = 240.0f;
        float modalX = windowWidth * 0.5f - modalW * 0.5f;
        float modalY = windowHeight * 0.5f - modalH * 0.5f;

        drawFilledRect(modalX, modalY, modalW, modalH, 0.10f, 0.05f, 0.05f, 0.95f);
        drawBorderRect(modalX, modalY, modalW, modalH, 3.0f, 1.0f, 0.2f, 0.2f);

        glColor3f(1.0f, 0.2f, 0.2f);
        drawText(modalX + 150.0f, modalY + modalH - 65.0f, "TIME EXPIRED!", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(0.9f, 0.8f, 0.8f);
        drawText(modalX + 130.0f, modalY + modalH - 115.0f, "Failed to reach the finish in time.", GLUT_BITMAP_HELVETICA_18);

        glColor3f(1.0f, 0.9f, 0.2f);
        drawText(modalX + 140.0f, modalY + 45.0f, "Press 'R' to Retry Game", GLUT_BITMAP_HELVETICA_18);
    }

    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void UIManager::drawDollCamFrame(float x, float y, float w, float h, float timeSec) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // Use current screen space
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    glOrtho(0.0, vp[2], 0.0, vp[3], -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Frame Border
    drawBorderRect(x, y, w, h, 2.5f, 0.15f, 0.85f, 0.95f, 0.95f); // Cyan border
    drawBorderRect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, 1.5f, 0.94f, 0.16f, 0.48f, 0.8f); // Pink outer trim

    // Corner brackets
    float cornerLen = 14.0f;
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    // Top-Left
    glVertex2f(x, y + h); glVertex2f(x + cornerLen, y + h);
    glVertex2f(x, y + h); glVertex2f(x, y + h - cornerLen);
    // Top-Right
    glVertex2f(x + w, y + h); glVertex2f(x + w - cornerLen, y + h);
    glVertex2f(x + w, y + h); glVertex2f(x + w, y + h - cornerLen);
    // Bottom-Left
    glVertex2f(x, y); glVertex2f(x + cornerLen, y);
    glVertex2f(x, y); glVertex2f(x, y + cornerLen);
    // Bottom-Right
    glVertex2f(x + w, y); glVertex2f(x + w - cornerLen, y);
    glVertex2f(x + w, y); glVertex2f(x + w, y + cornerLen);
    glEnd();

    // Top Header Banner inside PiP: "● LIVE REC" and "CAM 01: DOLL VIEW"
    drawFilledRect(x, y + h - 22.0f, w, 22.0f, 0.05f, 0.05f, 0.08f, 0.75f);

    // Blinking red recording dot
    bool recBlink = ((int)(timeSec * 2.0f) % 2 == 0);
    if (recBlink) {
        glColor3f(1.0f, 0.15f, 0.15f);
        drawFilledRect(x + 8.0f, y + h - 16.0f, 8.0f, 8.0f, 1.0f, 0.15f, 0.15f);
    }
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(x + 22.0f, y + h - 16.0f, "LIVE REC", GLUT_BITMAP_HELVETICA_10);
    drawText(x + w - 125.0f, y + h - 16.0f, "CAM 01 : DOLL SCAN", GLUT_BITMAP_HELVETICA_10);

    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
