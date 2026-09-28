#include "ui/UI.h"
#include "ui/UIPrimitives.h"
#include "core/Config.h"
#include <GL/glut.h>
#include <cstdio>
#include <cstring>

using namespace UIPrimitives;

void UIManager::init() {
}

// ---------------------------------------------------------------------------------------
// Left/Right sidebar backgrounds - solid panels reserved outside the main 3D viewport so
// the Doll Cam and the control list never overlap the gameplay view.
// ---------------------------------------------------------------------------------------
void UIManager::drawSidebarBackgrounds(int windowWidth, int windowHeight) {
    begin2D(windowWidth, windowHeight);

    // Left bar panel
    drawFilledRect(0.0f, 0.0f, (float)UI_LEFT_BAR_WIDTH, (float)windowHeight, 0.05f, 0.05f, 0.07f, 0.97f);

    // Right bar panel
    float rightX = (float)(windowWidth - UI_RIGHT_BAR_WIDTH);
    drawFilledRect(rightX, 0.0f, (float)UI_RIGHT_BAR_WIDTH, (float)windowHeight, 0.05f, 0.05f, 0.07f, 0.97f);

    // Neon divider lines separating the bars from the gameplay viewport
    glColor4f(0.94f, 0.16f, 0.48f, 0.55f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f((float)UI_LEFT_BAR_WIDTH, 0.0f);
    glVertex2f((float)UI_LEFT_BAR_WIDTH, (float)windowHeight);
    glVertex2f(rightX, 0.0f);
    glVertex2f(rightX, (float)windowHeight);
    glEnd();

    end2D();
}

// ---------------------------------------------------------------------------------------
// Left bar content: title header (Doll Cam feed is rendered separately by main.cpp into
// the reserved area below it), then the live stats stacked underneath the feed.
// ---------------------------------------------------------------------------------------
void UIManager::drawLeftBarStats(int windowWidth, int windowHeight, int pipBottomY,
                                  GameState gameState, LightState lightState,
                                  float gameTimer, int aliveCount, const Player& player) {
    begin2D(windowWidth, windowHeight);

    const float pad = 14.0f;
    const float contentW = (float)UI_LEFT_BAR_WIDTH - pad * 2.0f;

    // Header matches reference image: clean bold title and subline
    float headerY = (float)windowHeight - 16.0f;
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(pad, headerY - 14.0f, "SQUID GAME", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.70f, 0.70f, 0.78f);
    drawText(pad, headerY - 30.0f, "RED LIGHT, GREEN LIGHT", GLUT_BITMAP_HELVETICA_10);

    // ---- Stats stack, starting just below the Doll Cam feed ----
    float cy = (float)pipBottomY - 32.0f;

    drawHUDDigitalTimer(pad + contentW * 0.5f, cy, gameTimer);
    cy -= 46.0f;

    float boxH = 38.0f;
    drawFilledRect(pad, cy - boxH, contentW, boxH, 0.06f, 0.06f, 0.08f, 0.95f);
    drawBorderRect(pad, cy - boxH, contentW, boxH, 2.0f, 0.94f, 0.16f, 0.48f);
    char aliveBuf[64];
    sprintf(aliveBuf, "PLAYERS: %d / %d", aliveCount, TOTAL_CONTESTANTS);
    glColor3f(1.0f, 0.92f, 0.20f);
    drawText(pad + 10.0f, cy - boxH * 0.5f - 4.0f, aliveBuf, GLUT_BITMAP_HELVETICA_12);
    cy -= boxH + 12.0f;

    // Light status banner (shown at all times, green during wait/green, red during red)
    float bannerH = 38.0f;
    if (gameState == WAITING || lightState == GREEN_LIGHT) {
        drawFilledRect(pad, cy - bannerH, contentW, bannerH, 0.08f, 0.75f, 0.28f, 0.95f);
        drawBorderRect(pad, cy - bannerH, contentW, bannerH, 2.0f, 0.25f, 1.0f, 0.45f);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(pad + 8.0f, cy - bannerH * 0.5f - 4.0f, "GREEN LIGHT - MOVE!", GLUT_BITMAP_HELVETICA_12);
    } else {
        drawFilledRect(pad, cy - bannerH, contentW, bannerH, 0.85f, 0.08f, 0.08f, 0.95f);
        drawBorderRect(pad, cy - bannerH, contentW, bannerH, 2.0f, 1.0f, 0.3f, 0.3f);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(pad + 8.0f, cy - bannerH * 0.5f - 4.0f, "RED LIGHT - FREEZE!", GLUT_BITMAP_HELVETICA_12);
    }
    cy -= bannerH + 12.0f;

    // Progress bar to finish line
    float totalDist = FINISH_Z - START_Z;
    float currDist = player.z - START_Z;
    float progress = currDist / totalDist;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    glColor3f(0.85f, 0.85f, 0.90f);
    drawText(pad, cy, "COURSE PROGRESS", GLUT_BITMAP_HELVETICA_10);
    cy -= 16.0f;

    float pbarH = 14.0f;
    drawFilledRect(pad, cy - pbarH, contentW, pbarH, 0.15f, 0.15f, 0.18f, 0.9f);
    drawFilledRect(pad, cy - pbarH, contentW * progress, pbarH, 0.94f, 0.16f, 0.48f, 0.95f);
    drawBorderRect(pad, cy - pbarH, contentW, pbarH, 1.5f, 0.9f, 0.9f, 0.95f, 0.85f);

    char pctBuf[32];
    sprintf(pctBuf, "%d%% TO FINISH", (int)(progress * 100.0f));
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(pad, cy - pbarH - 14.0f, pctBuf, GLUT_BITMAP_HELVETICA_10);
    cy -= pbarH + 26.0f;

    // Speed pace badge
    float speedH = 32.0f;
    drawFilledRect(pad, cy - speedH, contentW, speedH, 0.06f, 0.06f, 0.08f, 0.90f);
    if (player.speedTier == SPEED_WALK) {
        drawBorderRect(pad, cy - speedH, contentW, speedH, 2.0f, 0.2f, 0.85f, 1.0f);
        glColor3f(0.3f, 0.9f, 1.0f);
        drawText(pad + 8.0f, cy - speedH * 0.5f - 4.0f, "PACE: WALK (Z)", GLUT_BITMAP_HELVETICA_10);
    } else if (player.speedTier == SPEED_SPRINT) {
        drawBorderRect(pad, cy - speedH, contentW, speedH, 2.0f, 1.0f, 0.35f, 0.15f);
        glColor3f(1.0f, 0.45f, 0.2f);
        drawText(pad + 8.0f, cy - speedH * 0.5f - 4.0f, "PACE: SPRINT (C)", GLUT_BITMAP_HELVETICA_10);
    } else {
        drawBorderRect(pad, cy - speedH, contentW, speedH, 2.0f, 0.15f, 0.95f, 0.35f);
        glColor3f(0.15f, 0.95f, 0.35f);
        drawText(pad + 8.0f, cy - speedH * 0.5f - 4.0f, "PACE: JOG (X)", GLUT_BITMAP_HELVETICA_10);
    }

    end2D();
}

// ---------------------------------------------------------------------------------------
// Right bar content: simplified, bold, high-contrast, with highlighted active modes
// ---------------------------------------------------------------------------------------
void UIManager::drawRightBarControls(int windowWidth, int windowHeight, CameraMode camMode) {
    begin2D(windowWidth, windowHeight);

    const float pad = 12.0f;
    const float barX = (float)(windowWidth - UI_RIGHT_BAR_WIDTH);
    const float contentW = (float)UI_RIGHT_BAR_WIDTH - pad * 2.0f;
    float y = (float)windowHeight - 16.0f;

    // Header
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(barX + pad, y - 10.0f, "GAME CONTROLS", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.70f, 0.70f, 0.78f);
    drawText(barX + pad, y - 24.0f, "CAMERA & MOVEMENT GUIDE", GLUT_BITMAP_HELVETICA_10);
    y -= 36.0f;

    // SECTION 1: CAMERAS (7 Clean Modes styled like attached image cards)
    struct CamOption { CameraMode mode; const char* key; const char* label; };
    const CamOption camList[] = {
        { CAM_THIRD_PERSON, "1", "3RD PERSON VIEW" },
        { CAM_OVERHEAD,     "2", "ARENA FREE 360° CAM" },
        { CAM_TOP_DOWN,     "3", "TOP ARENA VIEW" },
        { CAM_DOLL_CLOSEUP, "4", "DOLL 360° ORBIT" },
        { CAM_SIDE_LEFT,    "5", "LEFT BROADCAST" },
        { CAM_SIDE_RIGHT,   "6", "RIGHT BROADCAST" },
        { CAM_FINISH_LINE,  "7", "FINISH LINE 360°" }
    };

    const float btnH = 26.0f;
    const float btnSpacing = 4.0f;

    for (int i = 0; i < 7; ++i) {
        bool isActive = (camMode == camList[i].mode);
        char lineBuf[64];

        if (isActive) {
            // Exactly like GREEN LIGHT - MOVE! (Solid bright green, light green border, white text)
            drawFilledRect(barX + pad, y - btnH, contentW, btnH, 0.08f, 0.75f, 0.28f, 0.96f);
            drawBorderRect(barX + pad, y - btnH, contentW, btnH, 2.0f, 0.25f, 1.0f, 0.45f);
            glColor3f(1.0f, 1.0f, 1.0f);
            sprintf(lineBuf, ">> [%s] %s", camList[i].key, camList[i].label);
            drawText(barX + pad + 8.0f, y - btnH * 0.5f - 4.0f, lineBuf, GLUT_BITMAP_HELVETICA_10);
        } else {
            // Exactly like PLAYERS: 456 / 456 (Dark card, hot pink border, gold/yellow text)
            drawFilledRect(barX + pad, y - btnH, contentW, btnH, 0.06f, 0.06f, 0.08f, 0.94f);
            drawBorderRect(barX + pad, y - btnH, contentW, btnH, 1.8f, 0.94f, 0.16f, 0.48f);
            glColor3f(1.0f, 0.92f, 0.20f);
            sprintf(lineBuf, "   [%s] %s", camList[i].key, camList[i].label);
            drawText(barX + pad + 8.0f, y - btnH * 0.5f - 4.0f, lineBuf, GLUT_BITMAP_HELVETICA_10);
        }
        y -= btnH + btnSpacing;
    }
    y -= 8.0f;

    // SECTION 2: MOVEMENT CARD (Boxed Card with Cyan Border)
    float card1H = 52.0f;
    drawFilledRect(barX + pad, y - card1H, contentW, card1H, 0.06f, 0.06f, 0.08f, 0.94f);
    drawBorderRect(barX + pad, y - card1H, contentW, card1H, 1.8f, 0.15f, 0.85f, 0.95f);
    glColor3f(0.20f, 0.90f, 1.0f);
    drawText(barX + pad + 8.0f, y - 16.0f, "W / S : FORWARD / BACK", GLUT_BITMAP_HELVETICA_10);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(barX + pad + 8.0f, y - 34.0f, "A / D : TURN LEFT / RIGHT", GLUT_BITMAP_HELVETICA_10);
    y -= card1H + 8.0f;

    // SECTION 3: SPEED TIERS CARD (Boxed Card with Green Border)
    float card2H = 52.0f;
    drawFilledRect(barX + pad, y - card2H, contentW, card2H, 0.06f, 0.06f, 0.08f, 0.94f);
    drawBorderRect(barX + pad, y - card2H, contentW, card2H, 1.8f, 0.25f, 1.0f, 0.45f);
    glColor3f(0.30f, 1.0f, 0.50f);
    drawText(barX + pad + 8.0f, y - 16.0f, "Z / X / C : WALK / JOG / SPRINT", GLUT_BITMAP_HELVETICA_10);
    glColor3f(1.0f, 0.92f, 0.20f);
    drawText(barX + pad + 8.0f, y - 34.0f, "G : CYCLE SPEED GEAR", GLUT_BITMAP_HELVETICA_10);
    y -= card2H + 8.0f;

    // SECTION 4: MOUSE & UTILITY CARD (Boxed Card with Pink Border)
    float card3H = 68.0f;
    drawFilledRect(barX + pad, y - card3H, contentW, card3H, 0.06f, 0.06f, 0.08f, 0.94f);
    drawBorderRect(barX + pad, y - card3H, contentW, card3H, 1.8f, 0.94f, 0.16f, 0.48f);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(barX + pad + 8.0f, y - 16.0f, "MOUSE DRAG : 360° ORBIT", GLUT_BITMAP_HELVETICA_10);
    glColor3f(0.20f, 0.90f, 1.0f);
    drawText(barX + pad + 8.0f, y - 32.0f, "MOUSE WHEEL : ZOOM IN / OUT", GLUT_BITMAP_HELVETICA_10);
    glColor3f(1.0f, 0.92f, 0.20f);
    drawText(barX + pad + 8.0f, y - 48.0f, "V : CYCLE VIEWS | R : RESTART", GLUT_BITMAP_HELVETICA_10);

    end2D();
}

// ---------------------------------------------------------------------------------------
// Center-viewport overlay: while WAITING this is only a minimal start prompt (rules and
// controls already live permanently on the sidebars, so there is nothing else to read
// before playing). CAUGHT/WON/TIME_UP still show a full result modal.
// ---------------------------------------------------------------------------------------
void UIManager::renderStateOverlay(int windowWidth, int windowHeight, GameState gameState) {
    if (gameState == PLAYING) return;

    begin2D(windowWidth, windowHeight);

    int centerL = UI_LEFT_BAR_WIDTH;
    int centerR = windowWidth - UI_RIGHT_BAR_WIDTH;
    float centerViewW = (float)(centerR - centerL);
    if (centerViewW < 100.0f) { centerL = 0; centerR = windowWidth; centerViewW = (float)windowWidth; }
    float centerCX = (float)(centerL + centerR) * 0.5f;

    if (gameState == WAITING) {
        // Dim only the gameplay viewport - sidebars (Doll Cam + full controls) stay fully readable
        drawFilledRect((float)centerL, 0.0f, centerViewW, (float)windowHeight, 0.0f, 0.0f, 0.0f, 0.55f);

        float promptW = 460.0f;
        if (promptW > centerViewW - 40.0f) promptW = centerViewW - 40.0f;
        float promptH = 190.0f;
        float promptX = centerCX - promptW * 0.5f;
        float promptY = windowHeight * 0.5f - promptH * 0.5f;

        drawFilledRect(promptX, promptY, promptW, promptH, 0.06f, 0.06f, 0.09f, 0.96f);
        drawBorderRect(promptX, promptY, promptW, promptH, 3.0f, 0.94f, 0.16f, 0.48f);

        drawSquidGameSymbols(centerCX, promptY + promptH - 34.0f, 14.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(centerCX - 145.0f, promptY + promptH - 70.0f, "GAME 1: RED LIGHT, GREEN LIGHT", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.85f, 0.85f, 0.85f);
        drawText(centerCX - 155.0f, promptY + 78.0f, "Reach the finish line before the doll catches you.", GLUT_BITMAP_HELVETICA_12);
        drawText(centerCX - 155.0f, promptY + 58.0f, "Controls & the Doll Cam feed stay on the side bars.", GLUT_BITMAP_HELVETICA_12);

        float boxW = promptW - 60.0f;
        float boxH = 40.0f;
        float boxX = centerCX - boxW * 0.5f;
        float boxY = promptY + 18.0f;
        drawFilledRect(boxX, boxY, boxW, boxH, 0.94f, 0.16f, 0.48f, 0.25f);
        drawBorderRect(boxX, boxY, boxW, boxH, 2.0f, 0.94f, 0.16f, 0.48f);
        glColor3f(1.0f, 0.95f, 0.20f);
        drawText(centerCX - 148.0f, boxY + 14.0f, ">> PRESS SPACE OR CLICK TO START <<", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == CAUGHT) {
        drawFilledRect(0, 0, (float)windowWidth, (float)windowHeight, 0.5f, 0.0f, 0.0f, 0.45f);

        float modalW = 480.0f;
        float modalH = 260.0f;
        float modalX = centerCX - modalW * 0.5f;
        float modalY = windowHeight * 0.5f - modalH * 0.5f;

        drawFilledRect(modalX, modalY, modalW, modalH, 0.12f, 0.02f, 0.02f, 0.94f);
        drawBorderRect(modalX, modalY, modalW, modalH, 3.5f, 1.0f, 0.1f, 0.1f);

        glColor3f(1.0f, 0.15f, 0.15f);
        drawText(modalX + 85.0f, modalY + modalH - 65.0f, "PLAYER 456 - ELIMINATED!", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(0.9f, 0.8f, 0.8f);
        drawText(modalX + 95.0f, modalY + modalH - 115.0f, "Movement detected during RED LIGHT.", GLUT_BITMAP_HELVETICA_18);

        glColor3f(1.0f, 0.9f, 0.2f);
        drawText(modalX + 120.0f, modalY + 50.0f, "Press 'R' to Retry Game", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == WON) {
        drawFilledRect(0, 0, (float)windowWidth, (float)windowHeight, 0.1f, 0.35f, 0.2f, 0.35f);

        float modalW = 520.0f;
        float modalH = 280.0f;
        float modalX = centerCX - modalW * 0.5f;
        float modalY = windowHeight * 0.5f - modalH * 0.5f;

        drawFilledRect(modalX, modalY, modalW, modalH, 0.04f, 0.12f, 0.08f, 0.95f);
        drawBorderRect(modalX, modalY, modalW, modalH, 3.5f, 0.2f, 1.0f, 0.4f);

        glColor3f(0.2f, 1.0f, 0.4f);
        drawText(modalX + 55.0f, modalY + modalH - 60.0f, "CONGRATULATIONS! YOU SURVIVED!", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(1.0f, 0.92f, 0.2f);
        drawText(modalX + 110.0f, modalY + modalH - 115.0f, "PRIZE: 45.6 BILLION WON", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.9f, 0.9f, 0.9f);
        drawText(modalX + 100.0f, modalY + modalH - 155.0f, "You reached the finish line safely.", GLUT_BITMAP_HELVETICA_18);

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(modalX + 125.0f, modalY + 45.0f, "Press 'R' to Play Again", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == TIME_UP) {
        drawFilledRect(0, 0, (float)windowWidth, (float)windowHeight, 0.3f, 0.0f, 0.0f, 0.5f);

        float modalW = 460.0f;
        float modalH = 240.0f;
        float modalX = centerCX - modalW * 0.5f;
        float modalY = windowHeight * 0.5f - modalH * 0.5f;

        drawFilledRect(modalX, modalY, modalW, modalH, 0.10f, 0.05f, 0.05f, 0.95f);
        drawBorderRect(modalX, modalY, modalW, modalH, 3.0f, 1.0f, 0.2f, 0.2f);

        glColor3f(1.0f, 0.2f, 0.2f);
        drawText(modalX + 130.0f, modalY + modalH - 65.0f, "TIME EXPIRED!", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(0.9f, 0.8f, 0.8f);
        drawText(modalX + 110.0f, modalY + modalH - 115.0f, "Failed to reach the finish in time.", GLUT_BITMAP_HELVETICA_18);

        glColor3f(1.0f, 0.9f, 0.2f);
        drawText(modalX + 120.0f, modalY + 45.0f, "Press 'R' to Retry Game", GLUT_BITMAP_HELVETICA_18);
    }

    end2D();
}

void UIManager::drawDollCamFrame(float x, float y, float w, float h, float timeSec) {
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    begin2D(vp[2], vp[3]);

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
    drawFilledRect(x, y + h - 20.0f, w, 20.0f, 0.05f, 0.05f, 0.08f, 0.75f);

    // Blinking red recording dot
    bool recBlink = ((int)(timeSec * 2.0f) % 2 == 0);
    if (recBlink) {
        glColor3f(1.0f, 0.15f, 0.15f);
        drawFilledRect(x + 8.0f, y + h - 14.0f, 7.0f, 7.0f, 1.0f, 0.15f, 0.15f);
    }
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(x + 20.0f, y + h - 14.0f, "LIVE", GLUT_BITMAP_HELVETICA_10);
    drawText(x + w - 78.0f, y + h - 14.0f, "CAM 01: DOLL", GLUT_BITMAP_HELVETICA_10);

    end2D();
}
