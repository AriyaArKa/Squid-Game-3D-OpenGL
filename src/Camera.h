#ifndef CAMERA_H
#define CAMERA_H

#include "Types.h"

class CameraManager {
public:
    static void init();
    static void update(const Player& player, const Doll& doll, CameraMode mode, float dt);
    static void applyView(const Player& player, const Doll& doll, CameraMode mode);
    static void applyDollCloseupView(const Doll& doll);
    static void cycleMode(CameraMode& mode);

    // Mouse camera controls (360 Orbit & Zoom)
    static void handleMouseMotion(int x, int y);
    static void handleMouseClick(int button, int state, int x, int y);
    static void zoom(float delta, CameraMode mode);
    static void resetAnglesForMode(CameraMode mode);
};

#endif // CAMERA_H

