#include "engine/Primitives.h"
#include <cmath>
#include <map>
#include <memory>
#include <tuple>

namespace {
    const float PI = 3.14159265358979323846f;

    std::unique_ptr<Mesh> buildBox() {
        // 6 faces x 4 corners. Each face gets its own corners (not shared with neighbours)
        // so it keeps a flat normal, and its own (0,0)..(1,1) texture square.
        struct Face { glm::vec3 normal; glm::vec3 corners[4]; };
        const Face faces[6] = {
            {{ 0, 0, 1}, {{-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}}}, // Front (+Z)
            {{ 0, 0,-1}, {{ 0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}}}, // Back (-Z)
            {{-1, 0, 0}, {{-0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f,-0.5f}}}, // Left (-X)
            {{ 1, 0, 0}, {{ 0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f, 0.5f}}}, // Right (+X)
            {{ 0, 1, 0}, {{-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}}}, // Top (+Y)
            {{ 0,-1, 0}, {{-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f, 0.5f}, {-0.5f,-0.5f, 0.5f}}}, // Bottom (-Y)
        };
        const glm::vec2 uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

        std::vector<Vertex> v;
        std::vector<GLuint> idx;
        for (GLuint f = 0; f < 6; ++f) {
            for (int c = 0; c < 4; ++c) {
                v.push_back({faces[f].corners[c], faces[f].normal, {1, 1, 1}, uv[c]});
            }
            GLuint base = f * 4;
            idx.push_back(base + 0); idx.push_back(base + 1); idx.push_back(base + 2);
            idx.push_back(base + 0); idx.push_back(base + 2); idx.push_back(base + 3);
        }
        return std::make_unique<Mesh>(v, idx);
    }

    std::unique_ptr<Mesh> buildSphere(int stacks = 24, int slices = 24) {
        std::vector<Vertex> v;
        v.reserve((stacks + 1) * (slices + 1));
        for (int i = 0; i <= stacks; ++i) {
            float phi = -PI * 0.5f + PI * (float)i / (float)stacks; // -90..+90
            float y = sinf(phi);
            float r = cosf(phi);
            for (int j = 0; j <= slices; ++j) {
                float theta = 2.0f * PI * (float)j / (float)slices;
                glm::vec3 pos(r * cosf(theta), y, r * sinf(theta));
                glm::vec2 uv((float)j / (float)slices, (float)i / (float)stacks);
                v.push_back({pos, pos, {1,1,1}, uv});
            }
        }
        std::vector<GLuint> idx;
        int cols = slices + 1;
        for (int i = 0; i < stacks; ++i) {
            for (int j = 0; j < slices; ++j) {
                GLuint a = i * cols + j;
                GLuint b = a + cols;
                idx.push_back(a); idx.push_back(b); idx.push_back(a + 1);
                idx.push_back(a + 1); idx.push_back(b); idx.push_back(b + 1);
            }
        }
        return std::make_unique<Mesh>(v, idx);
    }

    // Side-only cylinder/cone frustum from z=0 (baseRadius) to z=height (topRadius), with
    // analytically correct slanted normals so tapered shapes (cones, funnels) still shade
    // right - not just a radial normal that ignores the taper.
    std::unique_ptr<Mesh> buildCylinder(float baseRadius, float topRadius, float height, int slices) {
        std::vector<Vertex> v;
        v.reserve((slices + 1) * 2);
        float dr = topRadius - baseRadius;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * (float)j / (float)slices;
            float c = cosf(theta), s = sinf(theta);
            glm::vec3 normal = glm::normalize(glm::vec3(height * c, height * s, -dr));
            float u = (float)j / (float)slices; // u wraps once around, v runs bottom -> top
            v.push_back({{baseRadius * c, baseRadius * s, 0.0f}, normal, {1,1,1}, {u, 0.0f}});
            v.push_back({{topRadius * c,  topRadius * s,  height}, normal, {1,1,1}, {u, 1.0f}});
        }
        std::vector<GLuint> idx;
        for (int j = 0; j < slices; ++j) {
            GLuint base = j * 2;
            GLuint next = base + 2;
            // base+0 = bottom ring here, base+1 = top ring here
            idx.push_back(base + 0); idx.push_back(next + 0); idx.push_back(base + 1);
            idx.push_back(base + 1); idx.push_back(next + 0); idx.push_back(next + 1);
        }
        return std::make_unique<Mesh>(v, idx);
    }

    // Flat annulus in the XY plane at z=0, facing +Z.
    std::unique_ptr<Mesh> buildDisk(float innerRadius, float outerRadius, int slices) {
        std::vector<Vertex> v;
        v.reserve((slices + 1) * 2);
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * (float)j / (float)slices;
            float c = cosf(theta), s = sinf(theta);
            // Planar mapping: the disk's bounding square becomes the (0,0)..(1,1) texture square.
            float k = 0.5f / outerRadius;
            v.push_back({{innerRadius * c, innerRadius * s, 0.0f}, {0,0,1}, {1,1,1},
                         {0.5f + innerRadius * c * k, 0.5f + innerRadius * s * k}});
            v.push_back({{outerRadius * c, outerRadius * s, 0.0f}, {0,0,1}, {1,1,1},
                         {0.5f + outerRadius * c * k, 0.5f + outerRadius * s * k}});
        }
        std::vector<GLuint> idx;
        for (int j = 0; j < slices; ++j) {
            GLuint base = j * 2;
            GLuint next = base + 2;
            idx.push_back(base + 0); idx.push_back(base + 1); idx.push_back(next + 1);
            idx.push_back(base + 0); idx.push_back(next + 1); idx.push_back(next + 0);
        }
        return std::make_unique<Mesh>(v, idx);
    }

    // Torus: tube radius = innerRadius, ring (major) radius = outerRadius, matching
    // glutSolidTorus(innerRadius, outerRadius, sides, rings)'s own parameter meaning.
    std::unique_ptr<Mesh> buildTorus(float tubeRadius, float ringRadius, int sides, int rings) {
        std::vector<Vertex> v;
        v.reserve((rings + 1) * (sides + 1));
        for (int i = 0; i <= rings; ++i) {
            float theta = 2.0f * PI * (float)i / (float)rings;
            float ct = cosf(theta), st = sinf(theta);
            for (int j = 0; j <= sides; ++j) {
                float phi = 2.0f * PI * (float)j / (float)sides;
                float cp = cosf(phi), sp = sinf(phi);
                glm::vec3 normal(cp * ct, cp * st, sp);
                glm::vec3 pos((ringRadius + tubeRadius * cp) * ct,
                               (ringRadius + tubeRadius * cp) * st,
                               tubeRadius * sp);
                v.push_back({pos, normal, {1,1,1}, {(float)i / (float)rings, (float)j / (float)sides}});
            }
        }
        std::vector<GLuint> idx;
        int cols = sides + 1;
        for (int i = 0; i < rings; ++i) {
            for (int j = 0; j < sides; ++j) {
                GLuint a = i * cols + j;
                GLuint b = a + cols;
                idx.push_back(a); idx.push_back(a + 1); idx.push_back(b);
                idx.push_back(b); idx.push_back(a + 1); idx.push_back(b + 1);
            }
        }
        return std::make_unique<Mesh>(v, idx);
    }
}

Mesh& Primitives::unitBox() {
    static std::unique_ptr<Mesh> mesh = buildBox();
    return *mesh;
}

Mesh& Primitives::unitSphere() {
    static std::unique_ptr<Mesh> mesh = buildSphere();
    return *mesh;
}

Mesh& Primitives::cylinder(float baseRadius, float topRadius, float height, int slices) {
    using Key = std::tuple<float, float, float, int>;
    static std::map<Key, std::unique_ptr<Mesh>> cache;
    Key key(baseRadius, topRadius, height, slices);
    auto it = cache.find(key);
    if (it == cache.end()) {
        it = cache.emplace(key, buildCylinder(baseRadius, topRadius, height, slices)).first;
    }
    return *it->second;
}

Mesh& Primitives::disk(float innerRadius, float outerRadius, int slices) {
    using Key = std::tuple<float, float, int>;
    static std::map<Key, std::unique_ptr<Mesh>> cache;
    Key key(innerRadius, outerRadius, slices);
    auto it = cache.find(key);
    if (it == cache.end()) {
        it = cache.emplace(key, buildDisk(innerRadius, outerRadius, slices)).first;
    }
    return *it->second;
}

Mesh& Primitives::torus(float innerRadius, float outerRadius, int sides, int rings) {
    using Key = std::tuple<float, float, int, int>;
    static std::map<Key, std::unique_ptr<Mesh>> cache;
    Key key(innerRadius, outerRadius, sides, rings);
    auto it = cache.find(key);
    if (it == cache.end()) {
        it = cache.emplace(key, buildTorus(innerRadius, outerRadius, sides, rings)).first;
    }
    return *it->second;
}
