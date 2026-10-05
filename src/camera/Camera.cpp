#include "camera/Camera.h"
#include "core/Config.h"
#include <GL/glut.h>
#include <cmath>

namespace {
    // Shared mouse orbit angles & distances
    float camYaw = 0.0f;       // Yaw angle in degrees
    float camPitch = 14.0f;    // Pitch angle in degrees
    float camDist = CAM_DISTANCE_3RD;      // Player 3rd person distance
    float dollCamDist = CAM_DISTANCE_DOLL; // Doll orbit distance (default 7.5 units)
    float droneDist = 58.0f;               // High stadium overview distance

    // Smooth interpolated camera coordinates
    float smoothCamX = 0.0f;
    float smoothCamY = 2.4f;
    float smoothCamZ = START_Z - CAM_DISTANCE_3RD;
    float smoothLookX = 0.0f;
    float smoothLookY = 1.4f;
    float smoothLookZ = START_Z;

    bool isMouseDown = false;
    int lastMouseX = 0;
    int lastMouseY = 0;

    const float PI = 3.14159265358979323846f;
    const float DEG2RAD = PI / 180.0f;
}

void CameraManager::init() {
    camYaw = 0.0f;
    camPitch = 14.0f;
    camDist = CAM_DISTANCE_3RD;
    dollCamDist = CAM_DISTANCE_DOLL;
    droneDist = 58.0f;

    smoothCamX = 0.0f;
    smoothCamY = CAM_HEIGHT_3RD;
    smoothCamZ = START_Z - CAM_DISTANCE_3RD;
    smoothLookX = 0.0f;
    smoothLookY = 1.3f;
    smoothLookZ = START_Z;
}

void CameraManager::resetAnglesForMode(CameraMode mode) {
    if (mode == CAM_DOLL_CLOSEUP) {
        camYaw = 0.0f;
        camPitch = 8.0f;
    } else if (mode == CAM_OVERHEAD) {
        camYaw = 0.0f;
        camPitch = 30.0f;
    } else if (mode == CAM_TOP_DOWN) {
        camYaw = 0.0f;
        camPitch = 90.0f;
    } else if (mode == CAM_FINISH_LINE) {
        camYaw = 0.0f;
        camPitch = 2.0f;
    } else if (mode == CAM_THIRD_PERSON) {
        camYaw = 0.0f;
        camPitch = 14.0f;
    }
}

void CameraManager::update(const Player& player, const Doll& doll, CameraMode mode, float dt) {
    float targetEyeX = 0.0f, targetEyeY = 0.0f, targetEyeZ = 0.0f;
    float targetLookX = 0.0f, targetLookY = 0.0f, targetLookZ = 0.0f;

    switch (mode) {
        case CAM_THIRD_PERSON: { // 1: 360° Orbit Follow around Player 456
            float totalYawRad = (player.facingAngle + camYaw) * DEG2RAD;
            float pitchRad = camPitch * DEG2RAD;
            float hDist = camDist * cosf(pitchRad);
            float vDist = camDist * sinf(pitchRad);

            targetEyeX = player.x - sinf(totalYawRad) * hDist;
            targetEyeZ = player.z - cosf(totalYawRad) * hDist;
            targetEyeY = player.y + 1.2f + vDist;

            // Clamping inside arena boundaries
            if (targetEyeX < -ARENA_HALF_WIDTH + 0.8f) targetEyeX = -ARENA_HALF_WIDTH + 0.8f;
            if (targetEyeX >  ARENA_HALF_WIDTH - 0.8f) targetEyeX =  ARENA_HALF_WIDTH - 0.8f;
            if (targetEyeZ < -ARENA_HALF_LENGTH + 0.8f) targetEyeZ = -ARENA_HALF_LENGTH + 0.8f;
            if (targetEyeZ >  ARENA_HALF_LENGTH - 0.8f) targetEyeZ =  ARENA_HALF_LENGTH - 0.8f;

            targetLookX = player.x + sinf(totalYawRad) * 4.5f;
            targetLookY = player.y + 1.3f;
            targetLookZ = player.z + cosf(totalYawRad) * 4.5f;
            break;
        }

        case CAM_OVERHEAD: { // 2: Arena Free 360° Orbit & Free Zoom Cam (Zoom in/out to any point!)
            targetLookX = 0.0f;
            targetLookY = 2.0f;
            targetLookZ = -3.5f; // Arena center

            float yawRad = camYaw * DEG2RAD;
            float pitchRad = (camPitch + 15.0f) * DEG2RAD;
            if (pitchRad < 0.05f) pitchRad = 0.05f;
            if (pitchRad > 1.48f) pitchRad = 1.48f;

            float hDist = droneDist * cosf(pitchRad);
            float vDist = droneDist * sinf(pitchRad);

            targetEyeX = targetLookX + sinf(yawRad) * hDist;
            targetEyeZ = targetLookZ - cosf(yawRad) * hDist;
            targetEyeY = targetLookY + vDist;
            break;
        }

        case CAM_TOP_DOWN: { // 3: Aerial Top View of entire system
            targetLookX = 0.0f;
            targetLookY = 0.0f;
            targetLookZ = -3.5f;

            targetEyeX = 0.0f;
            targetEyeY = 110.0f;
            targetEyeZ = -3.5f;
            break;
        }

        case CAM_DOLL_CLOSEUP: { // 4: Full 360° Orbit around Young-hee Doll & Surroundings
            targetLookX = doll.x;
            targetLookY = doll.y + 5.5f;
            targetLookZ = doll.z;

            float yawRad = camYaw * DEG2RAD;
            float pitchRad = camPitch * DEG2RAD;
            float hDist = dollCamDist * cosf(pitchRad);
            float vDist = dollCamDist * sinf(pitchRad);

            targetEyeX = doll.x + sinf(yawRad) * hDist;
            targetEyeZ = doll.z - cosf(yawRad) * hDist;
            targetEyeY = doll.y + 5.5f + vDist;

            if (targetEyeX < -ARENA_HALF_WIDTH + 0.8f) targetEyeX = -ARENA_HALF_WIDTH + 0.8f;
            if (targetEyeX >  ARENA_HALF_WIDTH - 0.8f) targetEyeX =  ARENA_HALF_WIDTH - 0.8f;
            if (targetEyeZ >  ARENA_HALF_LENGTH - 0.8f) targetEyeZ =  ARENA_HALF_LENGTH - 0.8f;
            if (targetEyeY < 0.6f) targetEyeY = 0.6f;
            break;
        }

        case CAM_SIDE_LEFT: { // 5: Left Broadcast Cam
            targetLookX = player.x;
            targetLookY = player.y + 1.2f;
            targetLookZ = player.z;

            targetEyeX = -ARENA_HALF_WIDTH + 1.5f;
            targetEyeY = 12.0f;
            targetEyeZ = player.z + 5.0f;
            break;
        }

        case CAM_SIDE_RIGHT: { // 6: Right Broadcast Cam
            targetLookX = player.x;
            targetLookY = player.y + 1.2f;
            targetLookZ = player.z;

            targetEyeX = ARENA_HALF_WIDTH - 1.5f;
            targetEyeY = 12.0f;
            targetEyeZ = player.z + 5.0f;
            break;
        }

        case CAM_FINISH_LINE: { // 7: Finish Line 360° Orbit Cam (from finish line looking at contestants)
            float posX = 0.0f;
            float posY = 2.4f;
            float posZ = FINISH_Z; // Standing right at finish line (Z = 45)

            // yaw = 0 looks toward -Z (straight down arena towards oncoming contestants)
            float totalYawRad = (180.0f + camYaw) * DEG2RAD;
            float pitchRad = camPitch * DEG2RAD;

            float forwardX = sinf(totalYawRad) * cosf(pitchRad);
            float forwardY = sinf(pitchRad);
            float forwardZ = cosf(totalYawRad) * cosf(pitchRad);

            targetEyeX = posX;
            targetEyeY = posY;
            targetEyeZ = posZ;

            targetLookX = posX + forwardX * 25.0f;
            targetLookY = posY + forwardY * 25.0f;
            targetLookZ = posZ + forwardZ * 25.0f;
            break;
        }

        default:
            targetLookX = player.x;
            targetLookY = player.y + 1.2f;
            targetLookZ = player.z;
            targetEyeX = player.x;
            targetEyeY = player.y + 2.2f;
            targetEyeZ = player.z - 5.0f;
            break;
    }

    // Smoothly interpolate camera position & target with exponential damping
    float interpSpeed = (mode == CAM_DOLL_CLOSEUP || mode == CAM_THIRD_PERSON) ? 12.0f : 8.0f;
    float factor = 1.0f - expf(-interpSpeed * dt);
    if (factor > 1.0f) factor = 1.0f;

    smoothCamX += (targetEyeX - smoothCamX) * factor;
    smoothCamY += (targetEyeY - smoothCamY) * factor;
    smoothCamZ += (targetEyeZ - smoothCamZ) * factor;

    smoothLookX += (targetLookX - smoothLookX) * factor;
    smoothLookY += (targetLookY - smoothLookY) * factor;
    smoothLookZ += (targetLookZ - smoothLookZ) * factor;
}

void CameraManager::applyView(const Player& player, const Doll& doll, CameraMode mode) {
    switch (mode) {
        case CAM_TOP_DOWN: // 3: Aerial Top View of entire system
            gluLookAt(0.0f, 110.0f, -3.5f,
                      0.0f, 0.0f, -3.5f,
                      0.0f, 0.0f, 1.0f);
            break;

        case CAM_THIRD_PERSON:
        case CAM_OVERHEAD:
        case CAM_DOLL_CLOSEUP:
        case CAM_SIDE_LEFT:
        case CAM_SIDE_RIGHT:
        case CAM_FINISH_LINE:
        default:
            gluLookAt(smoothCamX, smoothCamY, smoothCamZ,
                      smoothLookX, smoothLookY, smoothLookZ,
                      0.0f, 1.0f, 0.0f);
            break;
    }
}

void CameraManager::applyDollCloseupView(const Doll& doll) {
    // Iconic Episode 1 Camera: Over-the-shoulder behind the tree
    // Left side: Tree trunk and Doll's arm, face profile, and pigtail
    // Center & Right: Expansive sandy arena, 456 contestants, and horizon houses
    float eyeX = doll.x - 1.15f;
    float eyeY = doll.y + 4.90f;
    float eyeZ = doll.z + 2.10f;

    float lookX = doll.x + 0.40f;
    float lookY = doll.y + 2.20f;
    float lookZ = doll.z - 30.0f;

    gluLookAt(eyeX, eyeY, eyeZ,
              lookX, lookY, lookZ,
              0.0f, 1.0f, 0.0f);
}

void CameraManager::getViewVectors(const Player& player, const Doll& doll, CameraMode mode,
                                   float& eyeX, float& eyeY, float& eyeZ,
                                   float& lookX, float& lookY, float& lookZ,
                                   float& upX, float& upY, float& upZ, float& fov) {
    upX = 0.0f; upY = 1.0f; upZ = 0.0f;
    fov = 55.0f;

    switch (mode) {
        case CAM_THIRD_PERSON:
        case CAM_OVERHEAD:
        case CAM_DOLL_CLOSEUP:
        case CAM_SIDE_LEFT:
        case CAM_SIDE_RIGHT:
        case CAM_FINISH_LINE:
            eyeX = smoothCamX; eyeY = smoothCamY; eyeZ = smoothCamZ;
            lookX = smoothLookX; lookY = smoothLookY; lookZ = smoothLookZ;
            break;

        case CAM_TOP_DOWN:
            eyeX = 0.0f; eyeY = 110.0f; eyeZ = -3.5f;
            lookX = 0.0f; lookY = 0.0f; lookZ = -3.5f;
            upX = 0.0f; upY = 0.0f; upZ = 1.0f;
            fov = 58.0f;
            break;

        default:
            eyeX = smoothCamX; eyeY = smoothCamY; eyeZ = smoothCamZ;
            lookX = smoothLookX; lookY = smoothLookY; lookZ = smoothLookZ;
            break;
    }
}

void CameraManager::getDollCloseupVectors(const Doll& doll,
                                         float& eyeX, float& eyeY, float& eyeZ,
                                         float& lookX, float& lookY, float& lookZ,
                                         float& upX, float& upY, float& upZ, float& fov) {
    // Iconic Episode 1 Camera: Over-the-shoulder behind the tree
    // Left side: Tree trunk and Doll's arm, face profile, and pigtail
    // Center & Right: Expansive sandy arena, 456 contestants, and horizon houses
    eyeX = doll.x - 1.15f;
    eyeY = doll.y + 4.90f;
    eyeZ = doll.z + 2.10f;

    lookX = doll.x + 0.40f;
    lookY = doll.y + 2.20f;
    lookZ = doll.z - 30.0f;

    upX = 0.0f; upY = 1.0f; upZ = 0.0f;
    fov = 48.0f;
}

void CameraManager::cycleMode(CameraMode& mode) {
    static const CameraMode activeModes[] = {
        CAM_THIRD_PERSON,
        CAM_OVERHEAD,
        CAM_TOP_DOWN,
        CAM_DOLL_CLOSEUP,
        CAM_SIDE_LEFT,
        CAM_SIDE_RIGHT,
        CAM_FINISH_LINE
    };
    const int count = sizeof(activeModes) / sizeof(activeModes[0]);
    int currIdx = 0;
    for (int i = 0; i < count; ++i) {
        if (activeModes[i] == mode) {
            currIdx = i;
            break;
        }
    }
    mode = activeModes[(currIdx + 1) % count];
    resetAnglesForMode(mode);
}

void CameraManager::handleMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            isMouseDown = true;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            isMouseDown = false;
        }
    }
}

void CameraManager::handleMouseMotion(int x, int y) {
    if (isMouseDown) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        camYaw += dx * 0.35f;
        camPitch -= dy * 0.25f;

        while (camYaw > 360.0f) camYaw -= 360.0f;
        while (camYaw < -360.0f) camYaw += 360.0f;

        if (camPitch < -20.0f) camPitch = -20.0f;
        if (camPitch > 80.0f) camPitch = 80.0f;

        lastMouseX = x;
        lastMouseY = y;
    }
}

void CameraManager::zoom(float delta, CameraMode mode) {
    if (mode == CAM_DOLL_CLOSEUP) {
        dollCamDist += delta;
        if (dollCamDist < 2.0f) dollCamDist = 2.0f;
        if (dollCamDist > 38.0f) dollCamDist = 38.0f;
    } else if (mode == CAM_OVERHEAD) {
        droneDist += delta * 3.5f;
        if (droneDist < 3.0f) droneDist = 3.0f;
        if (droneDist > 140.0f) droneDist = 140.0f;
    } else {
        camDist += delta;
        if (camDist < 1.8f) camDist = 1.8f;
        if (camDist > 18.0f) camDist = 18.0f;
    }
}
