#include "glassbridge/GlassBridge.h"
#include <cmath>

namespace {
    const float PI = 3.14159265358979323846f;
}

// -----------------------------------------------------------------------------------
// CAMERA SYSTEM: ALL 7 ANGLES (STABLE & CLEAR NAVIGATION)
// -----------------------------------------------------------------------------------
void GlassBridgeManager::getCameraVectors(const std::vector<GBContestant>& contestants,
                                         int activeQueueIndex,
                                         GlassBridgeCamMode mode,
                                         float orbitYaw, float orbitPitch, float orbitDist,
                                         float& eyeX, float& eyeY, float& eyeZ,
                                         float& lookX, float& lookY, float& lookZ,
                                         float& upX, float& upY, float& upZ, float& fov) {
    upX = 0.0f; upY = 1.0f; upZ = 0.0f;
    fov = 52.0f;

    float focusX = 0.0f;
    float focusY = GB_BRIDGE_HEIGHT;
    float focusZ = 0.0f;

    if (activeQueueIndex < (int)contestants.size()) {
        const GBContestant& c = contestants[activeQueueIndex];
        focusX = c.x;
        focusZ = c.z;
    }

    switch (mode) {
        case GB_CAM_FOLLOW: {
            // Elevated 3rd Person Follow behind the active jumper / Player 456
            // Placed high enough to see over the player's head and clearly view both glass panels ahead
            eyeX = focusX * 0.30f;
            eyeY = GB_BRIDGE_HEIGHT + 2.85f;
            eyeZ = focusZ - 4.2f;

            lookX = focusX * 0.15f;
            lookY = GB_BRIDGE_HEIGHT + 0.65f;
            lookZ = focusZ + 13.5f;
            fov = 55.0f;
            break;
        }

        case GB_CAM_RUNWAY: {
            // Elevated runway perspective looking down the illuminated bridge towards exit
            eyeX = 0.0f;
            eyeY = GB_BRIDGE_HEIGHT + 4.8f;
            eyeZ = -5.8f;

            lookX = 0.0f;
            lookY = GB_BRIDGE_HEIGHT + 0.6f;
            lookZ = 28.0f;
            fov = 52.0f;
            break;
        }

        case GB_CAM_OVER_SHOULDER: {
            // Over-the-shoulder queue angle
            eyeX = 0.65f;
            eyeY = GB_BRIDGE_HEIGHT + 2.4f;
            eyeZ = focusZ - 3.6f;

            lookX = 0.0f;
            lookY = GB_BRIDGE_HEIGHT + 0.75f;
            lookZ = (focusZ > 3.0f) ? focusZ + 12.0f : 14.0f;
            fov = 50.0f;
            break;
        }

        case GB_CAM_ORBIT_360: {
            // Free 360° mouse drag orbit & wheel zoom
            float radYaw = orbitYaw * PI / 180.0f;
            float radPitch = orbitPitch * PI / 180.0f;

            eyeX = focusX + orbitDist * cosf(radPitch) * sinf(radYaw);
            eyeY = (GB_BRIDGE_HEIGHT + 1.2f) + orbitDist * sinf(radPitch);
            eyeZ = focusZ - orbitDist * cosf(radPitch) * cosf(radYaw);

            lookX = focusX;
            lookY = GB_BRIDGE_HEIGHT + 1.0f;
            lookZ = focusZ;
            fov = 50.0f;
            break;
        }

        case GB_CAM_TOP_DOWN: {
            // Aerial Plan View looking straight down
            eyeX = 0.0f;
            eyeY = GB_BRIDGE_HEIGHT + 24.0f;
            eyeZ = (focusZ > 8.0f) ? focusZ : 24.0f;

            lookX = 0.0f;
            lookY = GB_BRIDGE_HEIGHT;
            lookZ = eyeZ + 0.01f;

            upX = 0.0f; upY = 0.0f; upZ = 1.0f;
            fov = 60.0f;
            break;
        }

        case GB_CAM_SIDE_PROFILE: {
            // Stadium side broadcast showing drop altitude and leap arcs
            eyeX = -15.5f;
            eyeY = GB_BRIDGE_HEIGHT + 3.2f;
            eyeZ = (focusZ > 0.0f) ? focusZ : 16.0f;

            lookX = 0.0f;
            lookY = GB_BRIDGE_HEIGHT + 0.5f;
            lookZ = eyeZ;
            fov = 52.0f;
            break;
        }

        case GB_CAM_DESTINATION: {
            // Finish platform looking back at incoming contestants
            float finishZ = GB_BRIDGE_START_Z + (float)GB_NUM_STEPS * GB_STEP_LENGTH + 4.8f;
            eyeX = 0.0f;
            eyeY = GB_BRIDGE_HEIGHT + 2.8f;
            eyeZ = finishZ;

            lookX = focusX;
            lookY = GB_BRIDGE_HEIGHT + 1.0f;
            lookZ = focusZ;
            fov = 52.0f;
            break;
        }
    }
}
