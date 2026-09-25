#ifndef UI_H
#define UI_H

#include "core/Types.h"

class UIManager {
public:
    static void init();

    // Persistent sidebar chrome: drawn every frame during PLAYING (and behind the start
    // prompt while WAITING) so the Doll Cam feed and the full control list are always
    // visible and the player never has to pause or guess a key mid-run.
    static void drawSidebarBackgrounds(int windowWidth, int windowHeight);
    static void drawLeftBarStats(int windowWidth, int windowHeight, int pipBottomY,
                                  GameState gameState, LightState lightState,
                                  float gameTimer, int aliveCount, const Player& player);
    static void drawRightBarControls(int windowWidth, int windowHeight, CameraMode camMode);

    // Center-screen state overlay: minimal start prompt, or the eliminated/won/time-up result.
    static void renderStateOverlay(int windowWidth, int windowHeight, GameState gameState);

    static void drawDollCamFrame(float x, float y, float w, float h, float timeSec);
};

#endif // UI_H
