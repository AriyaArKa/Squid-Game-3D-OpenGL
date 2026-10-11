#include "tools/Showcase.h"
#include "engine/GLCompat.h"
#include "core/Config.h"
#include "core/Types.h"
#include "contestants/Contestants.h"
#include "guards/Guards.h"
#include "doll/Doll.h"
#include "doll/DollHead.h"
#include "doll/DollTree.h"
#include "environment/Environment.h"
#include "glassbridge/GlassBridge.h"
#include "render/LightFixtures.h"
#include <GL/glut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>
#include <cstdio>
#include <cmath>
#include <algorithm>

namespace {
    struct Item {
        std::string name;
        std::function<void()> draw;
        glm::vec3 viewDir;   // direction from the object toward the camera
        float explode;       // how far parts are pushed apart in the "parts" view
        bool wireframe;      // overlay the triangle edges (used for the basic shapes)
        glm::vec3 frontDir = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 sideDir  = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 topDir   = glm::vec3(0.0f, 1.0f, 0.0001f);
        glm::vec3 topUp    = glm::vec3(0.0f, 0.0f, -1.0f);
    };

    Doll showcaseDoll() {
        Doll d;
        DollManager::init(d);
        return d;
    }

    PointLightData lampAt(const glm::vec3& p) {
        PointLightData pt;
        pt.position = p;
        pt.color = glm::vec3(1.0f, 0.80f, 0.55f);
        return pt;
    }

    const std::vector<Item>& items() {
        static std::vector<Item> list;
        if (!list.empty()) return list;

        // ---- Basic shapes -------------------------------------------------------------
        list.push_back({"prim_box", [] { GLCompat::drawBox(1.0f, 1.0f, 1.0f); },
                        {1.0f, 0.75f, 1.35f}, 0.0f, true});
        list.push_back({"prim_sphere", [] { GLCompat::drawSphere(0.6f); },
                        {0.4f, 0.5f, 1.0f}, 0.0f, true});
        list.push_back({"prim_cylinder", [] {
                            GLCompat::pushMatrix();
                            GLCompat::translatef(0.0f, -0.6f, 0.0f);
                            GLCompat::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                            GLCompat::drawCylinder(0.45f, 0.45f, 1.2f, 24);
                            GLCompat::popMatrix();
                        }, {0.5f, 0.55f, 1.0f}, 0.0f, true});
        list.push_back({"prim_cone", [] {
                            GLCompat::pushMatrix();
                            GLCompat::translatef(0.0f, -0.6f, 0.0f);
                            GLCompat::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                            GLCompat::drawCone(0.5f, 1.2f, 24);
                            GLCompat::popMatrix();
                        }, {0.5f, 0.45f, 1.0f}, 0.0f, true});
        list.push_back({"prim_disk", [] {
                            GLCompat::pushMatrix();
                            GLCompat::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                            GLCompat::drawDisk(0.22f, 0.6f, 32);
                            GLCompat::popMatrix();
                        }, {0.4f, 1.1f, 1.0f}, 0.0f, true});
        list.push_back({"prim_torus", [] {
                            GLCompat::pushMatrix();
                            GLCompat::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                            GLCompat::drawTorus(0.16f, 0.46f, 16, 32);
                            GLCompat::popMatrix();
                        }, {0.3f, 0.9f, 1.0f}, 0.0f, true});
        list.push_back({"prim_polygon", [] {
                            GLCompat::setMaterial(1.0f, 1.0f, 1.0f);
                            glBegin(GL_TRIANGLES);           // a triangle (like the house gables)
                            glNormal3f(0.0f, 0.0f, 1.0f);
                            glVertex3f(-1.25f, -0.5f, 0.0f);
                            glVertex3f(-0.25f, -0.5f, 0.0f);
                            glVertex3f(-0.75f,  0.5f, 0.0f);
                            glEnd();
                            glBegin(GL_QUADS);               // a quad (split into 2 triangles)
                            glNormal3f(0.0f, 0.0f, 1.0f);
                            glVertex3f(0.15f, -0.5f, 0.0f);
                            glVertex3f(1.25f, -0.5f, 0.0f);
                            glVertex3f(1.05f,  0.5f, 0.0f);
                            glVertex3f(0.35f,  0.5f, 0.0f);
                            glEnd();
                        }, {0.25f, 0.2f, 1.0f}, 0.0f, true});

        // ---- Characters ---------------------------------------------------------------
        list.push_back({"player", [] {
                            ContestantsManager::drawCharacter(0, 0, 0, 0, 20.0f, -20.0f, PLAYER_NUMBER, true, 0.0f, 1.0f);
                        }, {0.75f, 0.35f, 1.0f}, 0.45f, false});
        list.push_back({"guard", [] {
                            Guard g{0, 0, 0, 0, GUARD_TRIANGLE, false, 0};
                            GuardsManager::drawSingleGuard(g);
                        }, {0.75f, 0.30f, 1.0f}, 0.40f, false});
        list.push_back({"gbguard", [] {
                            GlassBridgeManager::drawGuard(0, 0, 0, 0, 'C');
                        }, {0.75f, 0.30f, 1.0f}, 0.40f, false});
        list.push_back({"doll", [] {
                            Doll d = showcaseDoll();
                            d.x = d.y = d.z = 0.0f;
                            DollManager::drawDollOnly(d);
                        }, {0.70f, 0.20f, 1.0f}, 0.35f, false});
        list.push_back({"dollhead", [] {
                            Doll d = showcaseDoll();
                            DollHead::drawHead(d);
                        }, {0.55f, 0.10f, 1.0f}, 0.45f, false});
        list.push_back({"tree", [] { DollTree::drawDeadTree(0, 0, 0); },
                        {1.0f, 0.25f, 0.55f}, 0.25f, false});

        // ---- Round 1 scenery ----------------------------------------------------------
        list.push_back({"house", [] { EnvironmentManager::drawVillageHouse(0, 0, 0); },
                        {0.75f, 0.45f, 1.0f}, 0.30f, false});
        list.push_back({"clock", [] { EnvironmentManager::drawOverheadDigitalClock(272.0f); },
                        {-0.45f, 0.15f, -1.0f}, 0.25f, false,
                        {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, -0.0001f}, {0.0f, 0.0f, 1.0f}});
        list.push_back({"floodlight", [] {
                            LightFixtures::drawArenaFloodlight(lampAt(glm::vec3(0.0f, 16.0f, 0.0f)));
                        }, {0.6f, 0.25f, -1.0f}, 0.25f, false,
                        {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, -0.0001f}, {0.0f, 0.0f, 1.0f}});

        // ---- Round 2 (glass bridge) ---------------------------------------------------
        list.push_back({"bridge", [] {
                            std::vector<GlassStep> steps(3);
                            for (GlassStep& s : steps) s = {GLASS_LEFT, false, false, false, false};
                            std::vector<GBContestant> nobody;
                            GlassBridgeManager::drawBridge(steps, 0.0f, nobody, 0, 3);
                        }, {1.0f, 0.75f, -0.55f}, 0.22f, false,
                        {0.0f, 0.15f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0001f}, {0.0f, 0.0f, -1.0f}});
        list.push_back({"platform", [] { GlassBridgeManager::drawStartPlatform(); },
                        {0.8f, 0.55f, 1.0f}, 0.25f, false});
        list.push_back({"chandelier", [] {
                            LightFixtures::drawBridgeChandelier(lampAt(glm::vec3(0.0f)));
                        }, {0.3f, 0.45f, 1.0f}, 0.30f, false});
        list.push_back({"spotlight", [] {
                            SpotLightData sp;
                            sp.position = glm::vec3(0.0f);
                            sp.color = COLOR_GB_SPOT;
                            LightFixtures::drawFollowSpotlight(sp, false);
                        }, {0.5f, -0.35f, 1.0f}, 0.35f, false});
        list.push_back({"gantry", [] {
                            DirLightData d;
                            LightFixtures::updateSunDirection(d, 50.0f, 45.0f);
                            LightFixtures::drawOverheadFloodlightArray(d, 50.0f, 45.0f, false);
                        }, {0.6f, -0.45f, 1.0f}, 0.30f, false});
        list.push_back({"walllamp", [] {
                            LightFixtures::drawWallLamp(lampAt(glm::vec3(0.0f)), true, -1.0f);
                        }, {1.0f, 0.2f, 0.6f}, 0.35f, false});
        return list;
    }

    // Neutral "photo studio" light: one key light from the upper left of the camera.
    SceneLighting studioLighting(const glm::vec3& toCamera) {
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        glm::vec3 fwd = -glm::normalize(toCamera);
        glm::vec3 right = glm::normalize(glm::cross(fwd, std::fabs(fwd.y) > 0.95f ? glm::vec3(0, 0, 1) : up));
        glm::vec3 toLight = glm::normalize(-fwd * 0.9f + up * 1.1f - right * 0.6f);

        SceneLighting L;
        L.dir.enabled = true;
        L.dir.direction = -toLight;
        L.dir.color = glm::vec3(1.0f, 0.98f, 0.95f);
        L.dir.ambientIntensity = 0.62f;
        L.dir.diffuseIntensity = 0.78f;
        L.dir.specularIntensity = 0.55f;
        L.ambientSky = glm::vec3(0.80f, 0.82f, 0.88f);
        L.ambientGround = glm::vec3(0.50f, 0.47f, 0.45f);
        L.point.enabled = false;
        L.spot.enabled = false;
        L.fogEnabled = false;
        return L;
    }

    void measure(const Item& item, Shader& shader, Camera& camera, float explode,
                 const glm::vec3& center, glm::vec3& bmin, glm::vec3& bmax) {
        GLCompat::InspectSettings s;
        s.measureOnly = true;
        s.colorByShape = explode > 0.0f;
        s.explode = explode;
        s.explodeCenter = center;
        GLCompat::setInspect(s);
        GLCompat::beginFrame(shader, camera, studioLighting(item.viewDir));
        GLCompat::resetStats();
        item.draw();
        GLCompat::statsBounds(bmin, bmax);
        GLCompat::setInspect(GLCompat::InspectSettings());
    }

    // Moves the camera along viewDir until the box just fits the viewport (with a margin).
    void frameBox(Camera& camera, const glm::vec3& bmin, const glm::vec3& bmax,
                  const glm::vec3& viewDir, const glm::vec3& customUp, int w, int h, float fovDeg = 30.0f) {
        const float fov = fovDeg;
        const float margin = 0.88f;
        glm::vec3 dir = glm::normalize(viewDir);
        glm::vec3 up = customUp;
        if (glm::length(up) < 0.1f) {
            up = std::fabs(dir.y) > 0.95f ? glm::vec3(0, 0, -1) : glm::vec3(0, 1, 0);
        }
        glm::vec3 target = (bmin + bmax) * 0.5f;
        float radius = std::max(glm::length(bmax - bmin) * 0.5f, 0.05f);
        float dist = radius / sinf(glm::radians(fov * 0.5f));
        float aspect = (float)w / (float)h;
        camera.width = w;
        camera.height = h;

        for (int iter = 0; iter < 8; ++iter) {
            glm::vec3 eye = target + dir * dist;
            glm::mat4 view = glm::lookAt(eye, target, up);
            glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, dist * 0.01f, dist + radius * 4.0f);
            glm::vec2 lo(1e9f), hi(-1e9f);
            for (int c = 0; c < 8; ++c) {
                glm::vec3 corner((c & 1) ? bmax.x : bmin.x, (c & 2) ? bmax.y : bmin.y, (c & 4) ? bmax.z : bmin.z);
                glm::vec4 p = proj * view * glm::vec4(corner, 1.0f);
                glm::vec2 ndc(p.x / p.w, p.y / p.w);
                lo = glm::min(lo, ndc);
                hi = glm::max(hi, ndc);
            }
            // Re-center the picture, then scale the distance so the larger side fits.
            glm::vec2 mid = (lo + hi) * 0.5f;
            glm::vec3 camRight = glm::normalize(glm::cross(-dir, up));
            glm::vec3 camUp = glm::cross(camRight, -dir);
            float halfH = dist * tanf(glm::radians(fov * 0.5f));
            target += camRight * (mid.x * halfH * aspect) + camUp * (mid.y * halfH);
            float extent = std::max((hi.x - lo.x) * 0.5f, (hi.y - lo.y) * 0.5f);
            dist *= std::max(extent / margin, 0.2f);
        }
        camera.SetView(target + dir * dist, target, up, fov, dist * 0.01f, dist + radius * 6.0f);
    }
}

std::vector<std::string> Showcase::itemNames() {
    std::vector<std::string> names;
    for (const Item& it : items()) names.push_back(it.name);
    return names;
}

std::string Showcase::render(const std::string& name, bool parts, Shader& shader, Camera& camera,
                             int x, int y, int w, int h, const std::string& view) {
    const Item* item = nullptr;
    for (const Item& it : items()) {
        if (it.name == name) item = &it;
    }
    if (!item) return "";

    glViewport(x, y, w, h);

    // 1. Measure the assembled object, then (for the parts view) the exploded one.
    glm::vec3 bmin, bmax;
    measure(*item, shader, camera, 0.0f, glm::vec3(0.0f), bmin, bmax);
    glm::vec3 center = (bmin + bmax) * 0.5f;
    float explode = parts ? item->explode : 0.0f;
    if (explode > 0.0f) measure(*item, shader, camera, explode, center, bmin, bmax);

    // 2. Select viewing direction and camera framing based on view parameter
    glm::vec3 chosenDir = item->viewDir;
    glm::vec3 chosenUp = glm::vec3(0.0f);
    float chosenFov = 30.0f;

    if (view == "front") {
        chosenDir = item->frontDir;
        chosenFov = 15.0f;
    } else if (view == "side") {
        chosenDir = item->sideDir;
        chosenFov = 15.0f;
    } else if (view == "top") {
        chosenDir = item->topDir;
        chosenUp  = item->topUp;
        chosenFov = 15.0f;
    } else { // "iso" or default
        chosenDir = item->viewDir;
        chosenFov = 30.0f;
    }

    // 3. Frame it and draw it for real.
    frameBox(camera, bmin, bmax, chosenDir, chosenUp, w, h, chosenFov);
    GLCompat::InspectSettings s;
    s.colorByShape = parts;
    s.explode = explode;
    s.explodeCenter = center;
    GLCompat::setInspect(s);
    GLCompat::beginFrame(shader, camera, studioLighting(chosenDir));
    GLCompat::resetStats();
    item->draw();

    std::string counts;
    for (int k = 0; k < GLCompat::SHAPE_KIND_COUNT; ++k) {
        int n = GLCompat::shapeCount((GLCompat::ShapeKind)k);
        if (n > 0) {
            counts += GLCompat::shapeName((GLCompat::ShapeKind)k);
            counts += "=" + std::to_string(n) + "\n";
        }
    }

    // 3. Optional wireframe overlay showing the triangles each shape is made of.
    if (item->wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);
        GLCompat::InspectSettings wire = s;
        wire.forceColor = true;
        wire.forcedColor = glm::vec3(0.10f, 0.12f, 0.16f);
        GLCompat::setInspect(wire);
        item->draw();
        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    GLCompat::setInspect(GLCompat::InspectSettings());
    GLCompat::disableBlend();
    GLCompat::setDepthWrite(true);
    return counts;
}

bool Capture::saveBMP(const std::string& path, int x, int y, int w, int h) {
    std::vector<unsigned char> rgb((size_t)w * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(x, y, w, h, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());

    const int rowSize = (w * 3 + 3) & ~3;
    const unsigned int dataSize = (unsigned int)(rowSize * h);
    unsigned char header[54] = {'B', 'M'};
    auto put32 = [&](int at, unsigned int v) {
        for (int i = 0; i < 4; ++i) header[at + i] = (unsigned char)((v >> (8 * i)) & 0xFF);
    };
    put32(2, 54 + dataSize);
    put32(10, 54);
    put32(14, 40);
    put32(18, (unsigned int)w);
    put32(22, (unsigned int)h);   // positive height = bottom row first, same as glReadPixels
    header[26] = 1;
    header[28] = 24;
    put32(34, dataSize);

    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    fwrite(header, 1, 54, f);
    std::vector<unsigned char> row(rowSize, 0);
    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            const unsigned char* p = &rgb[((size_t)r * w + c) * 3];
            row[c * 3 + 0] = p[2];
            row[c * 3 + 1] = p[1];
            row[c * 3 + 2] = p[0];
        }
        fwrite(row.data(), 1, rowSize, f);
    }
    fclose(f);
    return true;
}
