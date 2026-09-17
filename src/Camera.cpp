#include "Camera.h"
#include "Config.h"
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
        camYaw = -52.0f; // Frames the iconic tree, resting arm, and doll face profile
        camPitch = 6.0f;
    } else if (mode == CAM_OVERHEAD) {
        camYaw = 0.0f;
        camPitch = 38.0f;
    } else if (mode == CAM_THIRD_PERSON) {
        camYaw = 0.0f;
        camPitch = 14.0f;
    } else if (mode == CAM_FIRST_PERSON) {
        camYaw = 0.0f;
        camPitch = 0.0f;
    } else if (mode == CAM_DOLL_POV) {
        camYaw = 0.0f;
        camPitch = -8.0f;
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

            targetLookX = player.x;
            targetLookY = player.y + 1.3f;
            targetLookZ = player.z + 0.5f;
            break;
        }

        case CAM_DOLL_CLOSEUP: { // 4: Full 360° Orbit around Young-hee Doll & Surroundings
            // Center of orbit is the Doll's head/chest center (Y + 5.5f)
            targetLookX = doll.x;
            targetLookY = doll.y + 5.5f;
            targetLookZ = doll.z;

            float yawRad = camYaw * DEG2RAD;
            float pitchRad = camPitch * DEG2RAD;
            float hDist = dollCamDist * cosf(pitchRad);
            float vDist = dollCamDist * sinf(pitchRad);

            // Orbit around Doll (yaw=0 is in front looking at face, yaw=180 looks at doll back and whole arena)
            targetEyeX = doll.x + sinf(yawRad) * hDist;
            targetEyeZ = doll.z - cosf(yawRad) * hDist;
            targetEyeY = doll.y + 5.5f + vDist;

            // Keep within arena bounds
            if (targetEyeX < -ARENA_HALF_WIDTH + 0.8f) targetEyeX = -ARENA_HALF_WIDTH + 0.8f;
            if (targetEyeX >  ARENA_HALF_WIDTH - 0.8f) targetEyeX =  ARENA_HALF_WIDTH - 0.8f;
            if (targetEyeZ >  ARENA_HALF_LENGTH - 0.8f) targetEyeZ =  ARENA_HALF_LENGTH - 0.8f;
            if (targetEyeY < 0.6f) targetEyeY = 0.6f;
            break;
        }

        case CAM_OVERHEAD: { // 0: 360° High Drone Stadium Overview
            targetLookX = 0.0f;
            targetLookY = 2.0f;
            targetLookZ = 0.0f; // Arena center

            float yawRad = camYaw * DEG2RAD;
            float pitchRad = (camPitch + 25.0f) * DEG2RAD;
            if (pitchRad > 1.45f) pitchRad = 1.45f;
            float hDist = droneDist * cosf(pitchRad);
            float vDist = droneDist * sinf(pitchRad);

            targetEyeX = sinf(yawRad) * hDist;
            targetEyeZ = -cosf(yawRad) * hDist;
            targetEyeY = 2.0f + vDist;
            break;
        }

        case CAM_TOP_DOWN: { // 3: Top-Down Aerial Floor View
            targetLookX = player.x * 0.4f;
            targetLookY = 0.0f;
            targetLookZ = (player.z + DOLL_Z) * 0.4f;

            targetEyeX = player.x * 0.4f;
            targetEyeY = 55.0f;
            targetEyeZ = (player.z + DOLL_Z) * 0.4f + 0.1f;
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

        case CAM_START_LINE: { // 8: Start Line
            targetLookX = 0.0f;
            targetLookY = 2.0f;
            targetLookZ = FINISH_Z;

            targetEyeX = 0.0f;
            targetEyeY = 2.5f;
            targetEyeZ = START_Z - 6.0f;
            break;
        }

        case CAM_FINISH_LINE: { // 9: Finish Line
            targetLookX = player.x;
            targetLookY = player.y + 1.2f;
            targetLookZ = player.z;

            targetEyeX = 0.0f;
            targetEyeY = 2.8f;
            targetEyeZ = FINISH_Z + 4.0f;
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

    // Smoothly interpolate camera position & target
    float interpSpeed = (mode == CAM_DOLL_CLOSEUP || mode == CAM_THIRD_PERSON) ? 14.0f : 8.0f;
    smoothCamX += (targetEyeX - smoothCamX) * interpSpeed * dt;
    smoothCamY += (targetEyeY - smoothCamY) * interpSpeed * dt;
    smoothCamZ += (targetEyeZ - smoothCamZ) * interpSpeed * dt;

    smoothLookX += (targetLookX - smoothLookX) * 16.0f * dt;
    smoothLookY += (targetLookY - smoothLookY) * 16.0f * dt;
    smoothLookZ += (targetLookZ - smoothLookZ) * 16.0f * dt;
}

void CameraManager::applyView(const Player& player, const Doll& doll, CameraMode mode) {
    switch (mode) {
        case CAM_THIRD_PERSON:
        case CAM_DOLL_CLOSEUP:
        case CAM_OVERHEAD:
        case CAM_SIDE_LEFT:
        case CAM_SIDE_RIGHT:
        case CAM_START_LINE:
        case CAM_FINISH_LINE:
            gluLookAt(smoothCamX, smoothCamY, smoothCamZ,
                      smoothLookX, smoothLookY, smoothLookZ,
                      0.0f, 1.0f, 0.0f);
            break;

        case CAM_FIRST_PERSON: { // 2: 1st Person Eyes with mouse look
            float totalYawRad = (player.facingAngle + camYaw) * DEG2RAD;
            float pitchRad = camPitch * DEG2RAD;

            float forwardX = sinf(totalYawRad) * cosf(pitchRad);
            float forwardY = sinf(pitchRad);
            float forwardZ = cosf(totalYawRad) * cosf(pitchRad);

            float eyeX = player.x;
            float eyeY = player.y + 1.65f;
            float eyeZ = player.z;

            gluLookAt(eyeX, eyeY, eyeZ,
                      eyeX + forwardX * 15.0f, eyeY + forwardY * 15.0f, eyeZ + forwardZ * 15.0f,
                      0.0f, 1.0f, 0.0f);
            break;
        }

        case CAM_TOP_DOWN: // 3: Top Floor Aerial Bird's Eye View
            gluLookAt(player.x * 0.3f, 55.0f, (player.z + DOLL_Z) * 0.4f,
                      player.x * 0.3f, 0.0f, (player.z + DOLL_Z) * 0.4f + 0.1f,
                      0.0f, 0.0f, 1.0f);
            break;

        case CAM_DOLL_POV: { // 7: Doll's Eyes Looking Down at contestants
            float headRad = (doll.headAngle + camYaw) * DEG2RAD;
            float lookDirX = sinf(headRad);
            float lookDirZ = -cosf(headRad);
            float pitchOffset = sinf(camPitch * DEG2RAD) * 8.0f;

            float dollEyeX = doll.x;
            float dollEyeY = doll.y + 6.0f;
            float dollEyeZ = doll.z;

            gluLookAt(dollEyeX, dollEyeY, dollEyeZ,
                      dollEyeX + lookDirX * 35.0f, dollEyeY - 4.5f + pitchOffset, dollEyeZ + lookDirZ * 35.0f,
                      0.0f, 1.0f, 0.0f);
            break;
        }

        default:
            gluLookAt(smoothCamX, smoothCamY, smoothCamZ,
                      smoothLookX, smoothLookY, smoothLookZ,
                      0.0f, 1.0f, 0.0f);
            break;
    }
}

void CameraManager::applyDollCloseupView(const Doll& doll) {
    // Iconic Episode 1 Tree-Side Close-up (matching the reference photo)
    // Looking past the tree trunk at the Doll's face, left arm touching the bark, and arena
    float eyeX = doll.x - 3.4f;
    float eyeY = doll.y + 5.5f;
    float eyeZ = doll.z + 3.0f;

    float lookX = doll.x - 0.4f;
    float lookY = doll.y + 5.3f;
    float lookZ = doll.z + 0.3f;

    gluLookAt(eyeX, eyeY, eyeZ,
              lookX, lookY, lookZ,
              0.0f, 1.0f, 0.0f);
}

void CameraManager::cycleMode(CameraMode& mode) {
    int next = ((int)mode) + 1;
    if (next > 9) next = 0;
    mode = (CameraMode)next;
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
    // Mouse wheel up/down handled via zoom() in main.cpp
}

void CameraManager::handleMouseMotion(int x, int y) {
    if (isMouseDown) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        camYaw += dx * 0.35f;
        camPitch -= dy * 0.25f;

        // Wrap yaw 0..360 for clean continuous rotation
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
        droneDist += delta * 2.0f;
        if (droneDist < 20.0f) droneDist = 20.0f;
        if (droneDist > 90.0f) droneDist = 90.0f;
    } else {
        camDist += delta;
        if (camDist < 1.8f) camDist = 1.8f;
        if (camDist > 18.0f) camDist = 18.0f;
    }
}
