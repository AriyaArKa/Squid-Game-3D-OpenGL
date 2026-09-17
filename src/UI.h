#ifndef UI_H
#define UI_H

#include "Types.h"

class UIManager {
public:
    static void init();
    static void renderHUD(int windowWidth, int windowHeight,
                          GameState gameState, LightState lightState,
                          float gameTimer, int aliveCount,
                          const Player& player, CameraMode camMode);
    static void drawDollCamFrame(float x, float y, float w, float h, float timeSec);
};

#endif // UI_H
