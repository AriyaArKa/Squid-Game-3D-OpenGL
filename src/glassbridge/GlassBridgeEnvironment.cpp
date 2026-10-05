#include "glassbridge/GlassBridge.h"
#include "engine/GLCompat.h"
#include <GL/glut.h>

// -----------------------------------------------------------------------------------
// 3D DRAW: AUTHENTIC SQUID GAME GLASS BRIDGE ENVIRONMENT (EPISODE 7)
// Clean, modular, realistic arena with dark chasm walls, safety net, and platforms
// -----------------------------------------------------------------------------------
void GlassBridgeManager::drawEnvironment(float animTimer) {
    const float bridgeY = GB_BRIDGE_HEIGHT; // 16.0f
    const float destZ = GB_BRIDGE_START_Z + (float)GB_NUM_STEPS * GB_STEP_LENGTH + 5.2f; // ~61.0f

    // -----------------------------------------------------------------------------------
    // 1. COLOSSAL INDUSTRIAL ARENA ENCLOSURE (Sleek dark chamber walls with neon accents)
    // -----------------------------------------------------------------------------------
    const float wallHalfW = 20.0f;
    const float wallMinZ  = -16.0f;
    const float wallMaxZ  =  72.0f;
    const float wallLen   = wallMaxZ - wallMinZ;
    const float wallCenterZ = (wallMinZ + wallMaxZ) * 0.5f;

    // Dark charcoal concrete / steel walls
    const float wallR = 0.07f, wallG = 0.08f, wallB = 0.11f;

    // (a) Left Wall
    GLCompat::pushMatrix();
    GLCompat::translatef(-wallHalfW, 10.0f, wallCenterZ);
    GLCompat::setMaterial(wallR, wallG, wallB, 15.0f);
    GLCompat::drawBox(0.4f, 48.0f, wallLen);
    // Horizontal Cyan Accent Neon Strip along wall at bridge height
    GLCompat::translatef(0.22f, bridgeY - 10.0f, 0.0f);
    GLCompat::setMaterial(0.0f, 0.85f, 1.0f, 60.0f);
    GLCompat::setEmissive(0.0f, 0.85f, 1.0f, 0.9f);
    GLCompat::drawBox(0.05f, 0.12f, wallLen);
    GLCompat::resetEmission();
    GLCompat::popMatrix();

    // (b) Right Wall
    GLCompat::pushMatrix();
    GLCompat::translatef(wallHalfW, 10.0f, wallCenterZ);
    GLCompat::setMaterial(wallR, wallG, wallB, 15.0f);
    GLCompat::drawBox(0.4f, 48.0f, wallLen);
    // Horizontal Hot Pink Accent Neon Strip along wall at bridge height
    GLCompat::translatef(-0.22f, bridgeY - 10.0f, 0.0f);
    GLCompat::setMaterial(0.95f, 0.15f, 0.50f, 60.0f);
    GLCompat::setEmissive(0.95f, 0.15f, 0.50f, 0.9f);
    GLCompat::drawBox(0.05f, 0.12f, wallLen);
    GLCompat::resetEmission();
    GLCompat::popMatrix();

    // (c) Far Back Wall (behind start platform)
    GLCompat::pushMatrix();
    GLCompat::translatef(0.0f, 10.0f, wallMinZ);
    GLCompat::setMaterial(wallR, wallG, wallB, 15.0f);
    GLCompat::drawBox(wallHalfW * 2.0f, 48.0f, 0.4f);
    GLCompat::popMatrix();

    // (d) Far Front Wall (behind destination platform)
    GLCompat::pushMatrix();
    GLCompat::translatef(0.0f, 10.0f, wallMaxZ);
    GLCompat::setMaterial(wallR, wallG, wallB, 15.0f);
    GLCompat::drawBox(wallHalfW * 2.0f, 48.0f, 0.4f);
    GLCompat::popMatrix();

    // (e) Deep Abyss Floor (Pitch black void far below)
    GLCompat::pushMatrix();
    GLCompat::translatef(0.0f, -14.0f, wallCenterZ);
    GLCompat::setMaterial(0.02f, 0.02f, 0.03f, 5.0f);
    GLCompat::drawBox(wallHalfW * 2.0f, 0.5f, wallLen);
    GLCompat::popMatrix();

    // -----------------------------------------------------------------------------------
    // 2. INDUSTRIAL SAFETY CARGO NETTING (Episode 7 iconic dark steel safety grid)
    // Suspended at Y = 2.0f (14 units below the bridge) across the void
    // -----------------------------------------------------------------------------------
    GLCompat::pushMatrix();
    const float netY = 2.0f;
    const float netMinZ = -2.0f;
    const float netMaxZ = 58.0f;
    const float netW = 14.0f;

    // Perimeter boundary cables for the cargo net
    GLCompat::setMaterial(0.25f, 0.26f, 0.28f, 40.0f);
    // Left boundary cable
    GLCompat::pushMatrix();
    GLCompat::translatef(-netW * 0.5f, netY, (netMinZ + netMaxZ) * 0.5f);
    GLCompat::drawBox(0.08f, 0.08f, netMaxZ - netMinZ);
    GLCompat::popMatrix();
    // Right boundary cable
    GLCompat::pushMatrix();
    GLCompat::translatef(netW * 0.5f, netY, (netMinZ + netMaxZ) * 0.5f);
    GLCompat::drawBox(0.08f, 0.08f, netMaxZ - netMinZ);
    GLCompat::popMatrix();

    // Longitudinal net cables
    GLCompat::setMaterial(0.18f, 0.19f, 0.22f, 25.0f);
    for (float nx = -netW * 0.5f + 1.4f; nx < netW * 0.5f; nx += 1.4f) {
        GLCompat::pushMatrix();
        GLCompat::translatef(nx, netY, (netMinZ + netMaxZ) * 0.5f);
        GLCompat::drawBox(0.03f, 0.03f, netMaxZ - netMinZ);
        GLCompat::popMatrix();
    }
    // Transverse net cables
    for (float nz = netMinZ; nz <= netMaxZ; nz += 2.2f) {
        GLCompat::pushMatrix();
        GLCompat::translatef(0.0f, netY, nz);
        GLCompat::drawBox(netW, 0.03f, 0.03f);
        GLCompat::popMatrix();
    }
    GLCompat::popMatrix();

    // -----------------------------------------------------------------------------------
    // 3. OVERHEAD INDUSTRIAL CEILING TRUSSES (Supporting the bridge high above)
    // -----------------------------------------------------------------------------------
    const float trussY = 28.5f;
    GLCompat::setMaterial(0.16f, 0.17f, 0.20f, 35.0f);
    for (float tz = 0.0f; tz <= 56.0f; tz += 14.0f) {
        // Transverse primary girder
        GLCompat::pushMatrix();
        GLCompat::translatef(0.0f, trussY, tz);
        GLCompat::drawBox(wallHalfW * 2.0f, 0.6f, 0.6f);
        GLCompat::popMatrix();

        // Vertical suspension cables dropping from truss toward bridge
        for (int side = -1; side <= 1; side += 2) {
            GLCompat::pushMatrix();
            GLCompat::translatef((float)side * 2.75f, (trussY + bridgeY) * 0.5f, tz);
            GLCompat::drawCylinder(0.02f, 0.02f, trussY - bridgeY, 6);
            GLCompat::popMatrix();
        }
    }

    // -----------------------------------------------------------------------------------
    // 4. STARTING PLATFORM (Queue area for contestants 1 to 16)
    // -----------------------------------------------------------------------------------
    GLCompat::pushMatrix();
    GLCompat::translatef(0.0f, bridgeY - 0.25f, -4.5f);

    // Platform concrete deck
    GLCompat::setMaterial(0.18f, 0.20f, 0.24f, 35.0f, 0.4f);
    GLCompat::drawBox(7.2f, 0.5f, 9.0f);

    // Hazard Yellow Caution Edge at the front jump line
    GLCompat::setMaterial(0.95f, 0.78f, 0.12f, 50.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.27f, 4.45f);
    GLCompat::drawBox(7.2f, 0.06f, 0.25f);
    glPopMatrix();

    // Perimeter safety railings (Left, Right, Back)
    GLCompat::setMaterial(0.85f, 0.15f, 0.45f, 40.0f); // Squid Game Pink railings
    for (int side = -1; side <= 1; side += 2) {
        float rx = (float)side * 3.5f;
        // Horizontal top bar
        glPushMatrix();
        glTranslatef(rx, 1.0f, 0.0f);
        GLCompat::drawBox(0.06f, 0.06f, 8.8f);
        glPopMatrix();
        // Vertical stanchions
        for (float pz = -4.0f; pz <= 4.0f; pz += 2.0f) {
            glPushMatrix();
            glTranslatef(rx, 0.5f, pz);
            GLCompat::drawBox(0.06f, 1.0f, 0.06f);
            glPopMatrix();
        }
    }

    // Entrance Archway from the Dormitory
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, -4.3f);
    // Outer pink frame
    GLCompat::setMaterial(0.92f, 0.16f, 0.48f, 40.0f);
    GLCompat::drawBox(4.4f, 4.6f, 0.35f);
    // Dark doorway recess
    GLCompat::setMaterial(0.05f, 0.05f, 0.07f, 15.0f);
    GLCompat::drawBox(2.6f, 3.8f, 0.42f);
    // Glowing pink arch strip
    GLCompat::setEmissive(0.95f, 0.18f, 0.52f, 1.2f);
    glPushMatrix();
    glTranslatef(0.0f, 1.95f, 0.20f);
    GLCompat::drawBox(2.8f, 0.10f, 0.05f);
    glPopMatrix();
    GLCompat::resetEmission();
    glPopMatrix();

    GLCompat::popMatrix();

    // Two Pink Guards at attention flanking the start ledge
    GlassBridgeManager::drawGuard(-2.4f, bridgeY, 0.2f, 30.0f, 'C');
    GlassBridgeManager::drawGuard( 2.4f, bridgeY, 0.2f, -30.0f, 'T');

    // -----------------------------------------------------------------------------------
    // 5. DESTINATION SANCTUARY PLATFORM (Where survivors land!)
    // -----------------------------------------------------------------------------------
    GLCompat::pushMatrix();
    GLCompat::translatef(0.0f, bridgeY - 0.25f, destZ);

    // Platform deck
    GLCompat::setMaterial(0.18f, 0.20f, 0.24f, 35.0f, 0.4f);
    GLCompat::drawBox(7.2f, 0.5f, 8.5f);

    // Hazard Yellow Caution Edge at the landing zone
    GLCompat::setMaterial(0.95f, 0.78f, 0.12f, 50.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.27f, -4.2f);
    GLCompat::drawBox(7.2f, 0.06f, 0.25f);
    glPopMatrix();

    // Exit Arch Portal
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, 4.0f);
    // Outer Hot Pink Arch
    GLCompat::setMaterial(0.92f, 0.16f, 0.48f, 40.0f);
    GLCompat::drawBox(4.4f, 4.6f, 0.35f);
    // Dark doorway recess
    GLCompat::setMaterial(0.05f, 0.06f, 0.08f, 15.0f);
    GLCompat::drawBox(2.6f, 3.8f, 0.42f);

    // Glowing Geometric Squid Game Badge: Circle ○ / Triangle △
    GLCompat::setMaterial(0.98f, 0.98f, 0.98f, 80.0f);
    GLCompat::setEmissive(0.98f, 0.98f, 0.98f, 1.2f);
    glPushMatrix();
    glTranslatef(0.0f, 2.05f, 0.20f);
    GLCompat::drawTorus(0.035f, 0.26f, 8, 16);
    glPopMatrix();
    GLCompat::resetEmission();

    glPopMatrix();
    GLCompat::popMatrix();

    // Two Guards flanking the Finish Exit
    GlassBridgeManager::drawGuard(-2.2f, bridgeY, destZ - 2.0f, 150.0f, 'S');
    GlassBridgeManager::drawGuard( 2.2f, bridgeY, destZ - 2.0f, -150.0f, 'T');
}
