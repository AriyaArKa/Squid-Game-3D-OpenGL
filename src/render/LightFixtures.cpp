#include "render/LightFixtures.h"
#include "engine/GLCompat.h"
#include "core/Config.h"
#include <GL/glut.h>
#include <cmath>

namespace LightFixtures {

    void updateSunDirection(DirLightData& dir, float sunElevation, float sunAzimuth) {
        float elRad = glm::radians(sunElevation);
        float azRad = glm::radians(sunAzimuth);
        float y = sinf(elRad);
        float x = cosf(elRad) * sinf(azRad);
        float z = cosf(elRad) * cosf(azRad);
        dir.direction = -glm::normalize(glm::vec3(x, y, z));
    }

    void drawArenaFloodlight(const PointLightData& pt) {
        glPushMatrix();
        glTranslatef(pt.position.x, 0.0f, pt.position.z);

        // 1. Tall Steel Lattice/Pylon Pole from ground y=0 to y=pt.position.y
        GLCompat::setMaterial(0.18f, 0.20f, 0.24f, 40.0f);
        glPushMatrix();
        glTranslatef(0.0f, pt.position.y * 0.5f, 0.0f);
        GLCompat::drawBox(0.45f, pt.position.y, 0.45f);
        glPopMatrix();

        // 2. Mounting crossbar at top
        glPushMatrix();
        glTranslatef(0.0f, pt.position.y, 0.0f);
        GLCompat::drawBox(2.4f, 0.25f, 0.6f);

        // 3. Floodlight reflector housing
        GLCompat::setMaterial(0.12f, 0.12f, 0.14f, 60.0f);
        glPushMatrix();
        glTranslatef(0.0f, -0.2f, 0.0f);
        GLCompat::drawBox(2.0f, 0.6f, 0.5f);
        glPopMatrix();

        // 4. Glowing Lamp Face
        if (pt.enabled) {
            GLCompat::setMaterial(1.0f, 0.95f, 0.80f, 100.0f);
            GLCompat::setEmissive(pt.color.r, pt.color.g, pt.color.b, 2.0f);
        } else {
            GLCompat::setMaterial(0.2f, 0.2f, 0.2f, 10.0f);
            GLCompat::resetEmission();
        }
        glPushMatrix();
        glTranslatef(0.0f, -0.25f, -0.26f);
        GLCompat::drawBox(1.8f, 0.45f, 0.08f);
        glPopMatrix();
        GLCompat::resetEmission();

        glPopMatrix();
        glPopMatrix();
    }

    void drawDirectionalSun(const DirLightData& dir, float sunElevation, float sunAzimuth, bool showSunRays) {
        float elRad = glm::radians(sunElevation);
        float azRad = glm::radians(sunAzimuth);
        float skyDist = 55.0f;
        float sunY = sinf(elRad) * skyDist;
        float sunX = cosf(elRad) * sinf(azRad) * skyDist;
        float sunZ = cosf(elRad) * cosf(azRad) * skyDist;

        glPushMatrix();
        glTranslatef(sunX, sunY, sunZ);

        if (dir.enabled) {
            // Luminous 3D Sun Sphere
            GLCompat::setMaterial(1.0f, 0.98f, 0.90f, 100.0f);
            GLCompat::setEmissive(dir.color.r * 1.5f, dir.color.g * 1.5f, dir.color.b * 1.2f, 3.0f);
            GLCompat::drawSphere(3.2f, 20, 20);

            // Radiant Corona Rings
            GLCompat::resetEmission();
            GLCompat::setUnlit(true);
            GLCompat::enableBlend();
            GLCompat::setDepthWrite(false);
            GLCompat::setAlpha(0.18f);
            GLCompat::setMaterial(1.0f, 0.88f, 0.40f, 10.0f);
            GLCompat::drawSphere(6.0f, 16, 16);
            GLCompat::setAlpha(0.05f);
            GLCompat::drawSphere(10.5f, 16, 16);

            // Directional Sun Rays pointing down to arena center
            if (showSunRays) {
                glm::vec3 rayDir = glm::normalize(glm::vec3(-sunX, -sunY, -sunZ));
                GLCompat::setAlpha(0.40f);
                GLCompat::setMaterial(1.0f, 0.90f, 0.50f, 5.0f);

                for (int seg = 1; seg <= 8; ++seg) {
                    glm::vec3 p = rayDir * ((float)seg * 5.0f);
                    glPushMatrix();
                    glTranslatef(p.x, p.y, p.z);
                    GLCompat::drawSphere(0.45f, 8, 8);
                    glPopMatrix();
                }

                // Wide translucent sunbeam cone
                GLCompat::setAlpha(0.08f);
                GLCompat::setMaterial(1.0f, 0.95f, 0.70f, 5.0f);
                glPushMatrix();
                float pitch = glm::degrees(asinf(rayDir.y));
                float yaw = glm::degrees(atan2f(rayDir.x, rayDir.z));
                glRotatef(yaw, 0.0f, 1.0f, 0.0f);
                glRotatef(-pitch + 90.0f, 1.0f, 0.0f, 0.0f);
                GLCompat::drawCone(12.0f, 40.0f, 16);
                glPopMatrix();
            }

            GLCompat::disableBlend();
            GLCompat::setDepthWrite(true);
            GLCompat::setUnlit(false);
            GLCompat::setAlpha(1.0f);
            GLCompat::resetEmission();
        } else {
            // Dormant Sun Sphere
            GLCompat::setMaterial(0.12f, 0.12f, 0.15f, 15.0f);
            GLCompat::resetEmission();
            GLCompat::drawSphere(3.2f, 16, 16);
        }

        glPopMatrix();
    }

    void drawBridgeChandelier(const PointLightData& pt) {
        glPushMatrix();
        glTranslatef(pt.position.x, pt.position.y, pt.position.z);

        // 4 Rigid suspension rigging cables extending up to ceiling
        GLCompat::setMaterial(0.22f, 0.24f, 0.26f, 40.0f);
        for (int c = 0; c < 4; ++c) {
            float ca = (float)c * 1.570796f;
            float cx = cosf(ca) * 1.5f;
            float cz = sinf(ca) * 1.5f;
            glPushMatrix();
            glTranslatef(cx, 2.75f, cz);
            GLCompat::drawBox(0.06f, 5.5f, 0.06f);
            glPopMatrix();
        }

        // Heavy industrial dual concentric octagonal chandelier rings
        GLCompat::setMaterial(0.28f, 0.25f, 0.20f, 60.0f);
        GLCompat::drawTorus(0.14f, 2.2f, 12, 24);
        GLCompat::drawTorus(0.12f, 1.3f, 10, 20);

        // 8 Perimeter warm bulb pendants around outer ring
        for (int i = 0; i < 8; ++i) {
            float ang = (float)i * 0.785398f;
            float bx = cosf(ang) * 2.2f;
            float bz = sinf(ang) * 2.2f;
            glPushMatrix();
            glTranslatef(bx, -0.22f, bz);
            if (pt.enabled) {
                GLCompat::setMaterial(1.0f, 0.92f, 0.70f, 100.0f);
                GLCompat::setEmissive(pt.color.r * 1.2f, pt.color.g * 1.2f, pt.color.b * 1.2f, 2.0f);
            } else {
                GLCompat::setMaterial(0.2f, 0.2f, 0.2f, 10.0f);
                GLCompat::resetEmission();
            }
            GLCompat::drawSphere(0.25f, 10, 10);
            glPopMatrix();
        }

        // Central Luminous Core
        if (pt.enabled) {
            GLCompat::setMaterial(1.0f, 0.96f, 0.82f, 100.0f);
            GLCompat::setEmissive(pt.color.r * 1.8f, pt.color.g * 1.6f, pt.color.b * 1.3f, 3.5f);
            GLCompat::drawSphere(0.95f, 20, 20);

            // Radiant Omnidirectional Amber Glow Halo
            GLCompat::resetEmission();
            GLCompat::setUnlit(true);
            GLCompat::enableBlend();
            GLCompat::setDepthWrite(false);
            GLCompat::setAlpha(0.04f);
            GLCompat::setMaterial(1.0f, 0.85f, 0.45f, 10.0f);
            GLCompat::drawSphere(2.8f, 16, 16);
            GLCompat::disableBlend();
            GLCompat::setDepthWrite(true);
            GLCompat::setUnlit(false);
            GLCompat::setAlpha(1.0f);
            GLCompat::resetEmission();
        } else {
            GLCompat::setMaterial(0.15f, 0.15f, 0.18f, 10.0f);
            GLCompat::resetEmission();
            GLCompat::drawSphere(0.95f, 16, 16);
        }

        glPopMatrix();
    }

    void drawFollowSpotlight(const SpotLightData& sp, bool showSunRays) {
        glPushMatrix();
        glTranslatef(sp.position.x, sp.position.y, sp.position.z);

        // Projector housing
        GLCompat::setMaterial(0.18f, 0.20f, 0.24f, 60.0f);
        glPushMatrix();
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GLCompat::drawCylinder(0.40f, 0.52f, 1.2f, 16);
        glPopMatrix();

        // Projector lens
        if (sp.enabled) {
            GLCompat::setMaterial(0.95f, 0.98f, 1.0f, 100.0f);
            GLCompat::setEmissive(sp.color.r * 1.8f, sp.color.g * 1.8f, sp.color.b * 1.8f, 3.0f);
            glPushMatrix();
            glTranslatef(0.0f, -1.22f, 0.0f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            GLCompat::drawDisk(0.0f, 0.50f, 16);
            glPopMatrix();

            // Downward Tracking Volumetric Spotlight Cone
            if (showSunRays) {
                GLCompat::resetEmission();
                GLCompat::setUnlit(true);
                GLCompat::enableBlend();
                GLCompat::setDepthWrite(false);
                GLCompat::setAlpha(0.08f);
                GLCompat::setMaterial(sp.color.r, sp.color.g, sp.color.b, 10.0f);
                glPushMatrix();
                glTranslatef(0.0f, -1.25f, 0.0f);
                glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
                GLCompat::drawCone(3.2f, 16.0f, 16);
                glPopMatrix();
                GLCompat::disableBlend();
                GLCompat::setDepthWrite(true);
                GLCompat::setUnlit(false);
                GLCompat::setAlpha(1.0f);
            }
            GLCompat::resetEmission();
        } else {
            GLCompat::setMaterial(0.2f, 0.2f, 0.2f, 10.0f);
            GLCompat::resetEmission();
            glPushMatrix();
            glTranslatef(0.0f, -1.22f, 0.0f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            GLCompat::drawDisk(0.0f, 0.50f, 16);
            glPopMatrix();
        }

        glPopMatrix();
    }

    void drawWallLamp(const PointLightData& pt, bool enabled, float side) {
        glPushMatrix();
        glTranslatef(pt.position.x, pt.position.y, pt.position.z);
        glRotatef(side * -90.0f, 0.0f, 1.0f, 0.0f); // local +z now points into the hall

        // Wall bracket reaching back to the wall
        GLCompat::setMaterial(COLOR_GB_STEEL_BEAM, 40.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -0.75f);
        GLCompat::drawBox(0.25f, 0.25f, 1.5f);
        glPopMatrix();

        // Lamp housing, tilted down toward the bridge
        glRotatef(25.0f, 1.0f, 0.0f, 0.0f);
        GLCompat::setMaterial(0.16f, 0.17f, 0.20f, 60.0f);
        GLCompat::drawBox(1.6f, 0.9f, 0.5f);

        // Glowing lamp face
        if (enabled) {
            GLCompat::setMaterial(1.0f, 0.97f, 0.90f, 100.0f);
            GLCompat::setEmissive(pt.color.r, pt.color.g, pt.color.b, 2.2f);
        } else {
            GLCompat::setMaterial(0.25f, 0.25f, 0.27f, 10.0f);
        }
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.27f);
        GLCompat::drawBox(1.4f, 0.7f, 0.05f);
        glPopMatrix();
        GLCompat::resetEmission();

        glPopMatrix();
    }

    void drawOverheadFloodlightArray(const DirLightData& dir, float sunElevation, float sunAzimuth, bool showSunRays) {
        // Place overhead stage light array on ceiling trusses directly above bridge
        glm::vec3 bridgeCenter(0.0f, GB_BRIDGE_HEIGHT, 28.0f);
        glm::vec3 normDir = glm::normalize(dir.direction);
        glm::vec3 lightOffset = -normDir * 18.0f;
        glm::vec3 lightSourcePos = bridgeCenter + lightOffset;
        lightSourcePos.y = 28.2f; // Structural ceiling truss level
        if (lightSourcePos.x < -16.0f) lightSourcePos.x = -16.0f;
        if (lightSourcePos.x >  16.0f) lightSourcePos.x =  16.0f;
        if (lightSourcePos.z <  -8.0f) lightSourcePos.z =  -8.0f;
        if (lightSourcePos.z >  64.0f) lightSourcePos.z =  64.0f;

        glPushMatrix();
        glTranslatef(lightSourcePos.x, lightSourcePos.y, lightSourcePos.z);

        // 1. Heavy Industrial Steel Stage Floodlight Gantry Rack
        GLCompat::setMaterial(0.18f, 0.20f, 0.24f, 40.0f);
        GLCompat::drawBox(8.0f, 1.2f, 3.2f);

        // 2. Structural safety rigging suspension arms up to ceiling
        GLCompat::setMaterial(0.12f, 0.14f, 0.16f, 30.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.95f, 0.0f);
        GLCompat::drawBox(7.2f, 0.8f, 0.4f);
        glPopMatrix();

        // 3. 4 Heavy Stage Floodlight Projectors across the gantry
        const float lampSpacing = 2.0f;
        for (int lamp = 0; lamp < 4; ++lamp) {
            float lx = -3.0f + (float)lamp * lampSpacing;
            glPushMatrix();
            glTranslatef(lx, -0.6f, 0.0f);

            // Projector body can
            GLCompat::setMaterial(0.14f, 0.16f, 0.18f, 50.0f);
            GLCompat::drawBox(1.5f, 0.8f, 1.8f);

            // Glowing Projector Lens / Reflector Dish
            glPushMatrix();
            glTranslatef(0.0f, -0.42f, 0.0f);
            if (dir.enabled) {
                GLCompat::setMaterial(1.0f, 0.98f, 0.90f, 100.0f);
                GLCompat::setEmissive(dir.color.r * 1.8f, dir.color.g * 1.8f, dir.color.b * 1.8f, 3.5f);
            } else {
                GLCompat::setMaterial(0.18f, 0.18f, 0.20f, 10.0f);
                GLCompat::resetEmission();
            }
            GLCompat::drawBox(1.35f, 0.08f, 1.6f);
            GLCompat::resetEmission();
            glPopMatrix();

            glPopMatrix();
        }

        // 4. Glowing Volumetric Stage Light Beams & Direction Indicator
        if (dir.enabled && showSunRays) {
            GLCompat::setUnlit(true);
            GLCompat::enableBlend();
            GLCompat::setDepthWrite(false);

            // (a) Broad theatrical light shaft along light vector down towards bridge
            GLCompat::setAlpha(0.15f);
            GLCompat::setMaterial(dir.color.r, dir.color.g, dir.color.b, 10.0f);
            glPushMatrix();
            glm::vec3 shaftVec = normDir * 20.0f;
            glTranslatef(shaftVec.x * 0.5f, shaftVec.y * 0.5f - 0.8f, shaftVec.z * 0.5f);
            GLCompat::drawBox(8.5f, 20.0f, 8.5f);
            glPopMatrix();

            // (b) 3D Directional Light Vector Pointer / Guide Arrow (bright cyan beacons)
            GLCompat::setAlpha(0.55f);
            GLCompat::setMaterial(0.0f, 0.88f, 1.0f, 10.0f);
            glPushMatrix();
            glTranslatef(0.0f, -1.0f, 0.0f);
            for (int seg = 1; seg <= 6; ++seg) {
                glm::vec3 arrowPos = normDir * ((float)seg * 2.8f);
                glPushMatrix();
                glTranslatef(arrowPos.x, arrowPos.y, arrowPos.z);
                GLCompat::drawSphere(0.24f, 8, 8);
                glPopMatrix();
            }
            glPopMatrix();

            GLCompat::disableBlend();
            GLCompat::setDepthWrite(true);
            GLCompat::setUnlit(false);
            GLCompat::setAlpha(1.0f);
        }

        glPopMatrix();
    }
}
