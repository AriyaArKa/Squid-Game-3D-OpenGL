#include "glassbridge/GlassBridge.h"
#include "engine/GLCompat.h"
#include <cmath>

// -----------------------------------------------------------------------------------
// 3D DRAW: BRIDGE STRUCTURE, STEEL TRUSSES & 16 GLASS PANELS
// -----------------------------------------------------------------------------------
void GlassBridgeManager::drawBridge(const std::vector<GlassStep>& steps, float animTimer,
                                    const std::vector<GBContestant>& contestants, int activeQueueIndex,
                                    int stepCount) {
    if (stepCount > (int)steps.size()) stepCount = (int)steps.size();
    float bridgeStart = GB_BRIDGE_START_Z - 1.0f; // ~2.0f
    float bridgeEnd = GB_BRIDGE_START_Z + (float)stepCount * GB_STEP_LENGTH + 0.5f; // ~56.3f for 16 steps
    float bridgeLength = bridgeEnd - bridgeStart;
    float bridgeCenterZ = (bridgeStart + bridgeEnd) * 0.5f;

    // Active contestant target step for floating visual guidance
    int targetStep = -1;
    if (activeQueueIndex < (int)contestants.size()) {
        targetStep = contestants[activeQueueIndex].currentStep + 1;
    }

    // -----------------------------------------------------------------------------------
    // 1. TWO PARALLEL HEAVY STEEL BEAMS & NEON RUNNER RAILS (Left & Right)
    // -----------------------------------------------------------------------------------
    for (int side = -1; side <= 1; side += 2) {
        float bx = (float)side * 2.75f;

        // (a) Lower Heavy Steel Stringer (I-beam carrying the bridge)
        GLCompat::setMaterial(COLOR_GB_STEEL_BEAM, 64.0f);
        GLCompat::pushMatrix();
        GLCompat::translatef(bx, GB_BRIDGE_HEIGHT - 0.22f, bridgeCenterZ);
        GLCompat::drawBox(0.24f, 0.42f, bridgeLength);
        GLCompat::popMatrix();

        // (b) Upper Tubular Steel Handrail
        GLCompat::setMaterial(COLOR_GB_STEEL_LIGHT, 78.0f);
        GLCompat::pushMatrix();
        GLCompat::translatef(bx, GB_BRIDGE_HEIGHT + 1.05f, bridgeCenterZ);
        GLCompat::drawBox(0.08f, 0.08f, bridgeLength);
        GLCompat::popMatrix();

        // (c) Continuous Sleek Squid Game Hot Pink Neon Runner Strip on the handrail
        GLCompat::setMaterial(COLOR_GB_PINK_RAIL, 60.0f);
        GLCompat::setEmissive(COLOR_GB_PINK_RAIL.r, COLOR_GB_PINK_RAIL.g, COLOR_GB_PINK_RAIL.b, 1.2f);
        GLCompat::pushMatrix();
        GLCompat::translatef(bx - (float)side * 0.06f, GB_BRIDGE_HEIGHT + 1.02f, bridgeCenterZ);
        GLCompat::drawBox(0.03f, 0.03f, bridgeLength);
        GLCompat::popMatrix();
        GLCompat::resetEmission();

        // (d) Vertical Stanchions at each of the 16 steps
        GLCompat::setMaterial(COLOR_GB_STEEL_LIGHT, 56.0f);
        for (int i = 0; i <= stepCount; ++i) {
            float pz = GB_BRIDGE_START_Z + (float)i * GB_STEP_LENGTH - GB_STEP_LENGTH * 0.5f;
            GLCompat::pushMatrix();
            GLCompat::translatef(bx, GB_BRIDGE_HEIGHT + 0.45f, pz);
            GLCompat::drawBox(0.06f, 1.05f, 0.06f);
            GLCompat::popMatrix();
        }
    }

    // -----------------------------------------------------------------------------------
    // 2. CENTRAL STEEL DIVIDER SPINE (Between Left and Right glass panes)
    // -----------------------------------------------------------------------------------
    GLCompat::setMaterial(COLOR_GB_STEEL_BEAM, 58.0f);
    GLCompat::pushMatrix();
    GLCompat::translatef(0.0f, GB_BRIDGE_HEIGHT - 0.22f, bridgeCenterZ);
    GLCompat::drawBox(0.18f, 0.35f, bridgeLength);
    GLCompat::popMatrix();

    // -----------------------------------------------------------------------------------
    // 3. THE 16 GLASS STEP PAIRS
    // -----------------------------------------------------------------------------------
    for (int i = 0; i < stepCount; ++i) {
        float sz = GB_BRIDGE_START_Z + (float)i * GB_STEP_LENGTH;
        const GlassStep& step = steps[i];
        bool isTargetForActive = (i == targetStep);

        // Transverse Steel Crossbeam supporting this step
        GLCompat::setMaterial(COLOR_GB_STEEL_BEAM, 60.0f);
        GLCompat::pushMatrix();
        GLCompat::translatef(0.0f, GB_BRIDGE_HEIGHT - 0.25f, sz);
        GLCompat::drawBox(5.6f, 0.20f, 0.35f);
        GLCompat::popMatrix();

        // Left & Right Glass Panels
        for (int sideIdx = 0; sideIdx < 2; ++sideIdx) {
            GlassSide side = (sideIdx == 0) ? GLASS_LEFT : GLASS_RIGHT;
            float px = (side == GLASS_LEFT) ? GB_LEFT_PANEL_X : GB_RIGHT_PANEL_X;
            bool isShattered = (side == GLASS_LEFT) ? step.leftShattered : step.rightShattered;
            bool isRevealed = (side == GLASS_LEFT) ? step.leftRevealed : step.rightRevealed;

            // Rubber Mounting Corner Clamps
            GLCompat::setMaterial(0.10f, 0.10f, 0.12f, 40.0f);
            for (int dx = -1; dx <= 1; dx += 2) {
                for (int dz = -1; dz <= 1; dz += 2) {
                    GLCompat::pushMatrix();
                    GLCompat::translatef(px + (float)dx * (GB_PANEL_WIDTH * 0.47f),
                                         GB_BRIDGE_HEIGHT - 0.04f,
                                         sz + (float)dz * (GB_PANEL_LENGTH * 0.47f));
                    GLCompat::drawBox(0.16f, 0.12f, 0.16f);
                    GLCompat::popMatrix();
                }
            }

            if (!isShattered) {
                GLCompat::enableBlend();
                GLCompat::setDepthWrite(false);

                // Polished translucent tempered glass slab
                GLCompat::setMaterial(COLOR_GB_GLASS, 120.0f, 1.0f);
                GLCompat::setAlpha(0.42f);

                GLCompat::pushMatrix();
                GLCompat::translatef(px, GB_BRIDGE_HEIGHT - 0.02f, sz);
                GLCompat::drawBox(GB_PANEL_WIDTH, GB_PANEL_THICKNESS, GB_PANEL_LENGTH);
                GLCompat::popMatrix();

                // Revealed Safe Panel: Emerald Green Glow
                if (isRevealed) {
                    GLCompat::setEmissive(COLOR_GB_SAFE.r, COLOR_GB_SAFE.g, COLOR_GB_SAFE.b, 1.2f);
                    GLCompat::setAlpha(0.70f);
                    GLCompat::pushMatrix();
                    GLCompat::translatef(px, GB_BRIDGE_HEIGHT + 0.04f, sz);
                    GLCompat::drawBox(GB_PANEL_WIDTH * 0.96f, 0.02f, GB_PANEL_LENGTH * 0.96f);
                    GLCompat::popMatrix();
                    GLCompat::resetEmission();
                }

                // Active Jumper's Frontier Choice: Clean, glowing direction badge
                if (isTargetForActive) {
                    float pulse = sinf(animTimer * 5.0f) * 0.20f + 0.80f;
                    if (side == GLASS_LEFT) {
                        // Cyan [A: LEFT] Marker
                        GLCompat::setEmissive(0.0f, 0.88f * pulse, 1.0f * pulse, 1.2f);
                        GLCompat::setAlpha(0.85f);
                        GLCompat::pushMatrix();
                        GLCompat::translatef(px, GB_BRIDGE_HEIGHT + 0.05f, sz);
                        GLCompat::drawBox(GB_PANEL_WIDTH * 0.92f, 0.02f, GB_PANEL_LENGTH * 0.92f);
                        // Clean floating indicator badge
                        GLCompat::translatef(0.0f, 0.25f, 0.0f);
                        GLCompat::drawBox(0.60f, 0.20f, 0.05f);
                        GLCompat::popMatrix();
                        GLCompat::resetEmission();
                    } else {
                        // Hot Pink [D: RIGHT] Marker
                        GLCompat::setEmissive(1.0f * pulse, 0.15f * pulse, 0.50f * pulse, 1.2f);
                        GLCompat::setAlpha(0.85f);
                        GLCompat::pushMatrix();
                        GLCompat::translatef(px, GB_BRIDGE_HEIGHT + 0.05f, sz);
                        GLCompat::drawBox(GB_PANEL_WIDTH * 0.92f, 0.02f, GB_PANEL_LENGTH * 0.92f);
                        // Clean floating indicator badge
                        GLCompat::translatef(0.0f, 0.25f, 0.0f);
                        GLCompat::drawBox(0.60f, 0.20f, 0.05f);
                        GLCompat::popMatrix();
                        GLCompat::resetEmission();
                    }
                }

                GLCompat::setAlpha(1.0f);
                GLCompat::disableBlend();
                GLCompat::setDepthWrite(true);
            } else {
                // Shattered Panel: Clean red warning edge around the empty frame opening
                GLCompat::enableBlend();
                GLCompat::setDepthWrite(false);
                GLCompat::setEmissive(0.95f, 0.15f, 0.15f, 0.9f);
                GLCompat::setAlpha(0.55f);

                GLCompat::pushMatrix();
                GLCompat::translatef(px, GB_BRIDGE_HEIGHT, sz);
                for (int dx = -1; dx <= 1; dx += 2) {
                    for (int dz = -1; dz <= 1; dz += 2) {
                        GLCompat::pushMatrix();
                        GLCompat::translatef((float)dx * (GB_PANEL_WIDTH * 0.42f), 0.0f, (float)dz * (GB_PANEL_LENGTH * 0.42f));
                        GLCompat::drawBox(0.24f, 0.02f, 0.24f);
                        GLCompat::popMatrix();
                    }
                }
                GLCompat::popMatrix();
                GLCompat::resetEmission();
                GLCompat::setAlpha(1.0f);
                GLCompat::disableBlend();
                GLCompat::setDepthWrite(true);
            }
        }
    }
}
