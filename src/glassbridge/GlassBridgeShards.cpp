#include "glassbridge/GlassBridge.h"
#include "engine/GLCompat.h"
#include <cstdlib>

// -----------------------------------------------------------------------------------
// SPWAN GLASS SHARDS FX (Particle simulation of breaking tempered glass)
// -----------------------------------------------------------------------------------
void GlassBridgeManager::spawnShards(std::vector<GlassShard>& shards, float centerX, float centerY, float centerZ) {
    int numShards = 55;
    for (int i = 0; i < numShards; ++i) {
        GlassShard s;
        s.x = centerX + (((float)rand() / RAND_MAX) - 0.5f) * GB_PANEL_WIDTH;
        s.y = centerY + (((float)rand() / RAND_MAX) - 0.5f) * 0.15f;
        s.z = centerZ + (((float)rand() / RAND_MAX) - 0.5f) * GB_PANEL_LENGTH;

        s.vx = (((float)rand() / RAND_MAX) - 0.5f) * 7.5f;
        s.vy = 2.0f + ((float)rand() / RAND_MAX) * 4.5f;
        s.vz = (((float)rand() / RAND_MAX) - 0.5f) * 7.5f;

        s.rx = ((float)rand() / RAND_MAX) * 360.0f;
        s.ry = ((float)rand() / RAND_MAX) * 360.0f;
        s.rz = ((float)rand() / RAND_MAX) * 360.0f;

        s.vrx = (((float)rand() / RAND_MAX) - 0.5f) * 600.0f;
        s.vry = (((float)rand() / RAND_MAX) - 0.5f) * 600.0f;
        s.vrz = (((float)rand() / RAND_MAX) - 0.5f) * 600.0f;

        s.size = 0.08f + ((float)rand() / RAND_MAX) * 0.16f;

        s.r = 0.70f + ((float)rand() / RAND_MAX) * 0.25f;
        s.g = 0.88f + ((float)rand() / RAND_MAX) * 0.12f;
        s.b = 0.98f;
        s.a = 0.85f;
        s.life = 2.0f + ((float)rand() / RAND_MAX) * 0.8f;

        shards.push_back(s);
    }
}

// -----------------------------------------------------------------------------------
// 3D DRAW: TUMBLING GLASS SHARDS
// -----------------------------------------------------------------------------------
void GlassBridgeManager::drawShards(const std::vector<GlassShard>& shards) {
    if (shards.empty()) return;

    GLCompat::enableBlend();
    GLCompat::setDepthWrite(false);

    for (const auto& s : shards) {
        GLCompat::pushMatrix();
        GLCompat::translatef(s.x, s.y, s.z);
        GLCompat::rotatef(s.rx, 1.0f, 0.0f, 0.0f);
        GLCompat::rotatef(s.ry, 0.0f, 1.0f, 0.0f);
        GLCompat::rotatef(s.rz, 0.0f, 0.0f, 1.0f);

        GLCompat::setMaterial(s.r, s.g, s.b, 90.0f, 1.0f);
        GLCompat::setEmissive(s.r * 0.6f, s.g * 0.8f, s.b * 1.1f, 0.7f);
        GLCompat::setAlpha(s.a);

        GLCompat::drawBox(s.size, s.size * 0.35f, s.size * 1.5f);
        GLCompat::popMatrix();
    }

    GLCompat::resetEmission();
    GLCompat::setAlpha(1.0f);
    GLCompat::disableBlend();
    GLCompat::setDepthWrite(true);
}
