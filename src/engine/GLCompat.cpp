#include "engine/GLCompat.h"
#include "engine/Primitives.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <vector>
#include <memory>
#include <cmath>
#include <string>
#include <algorithm>

namespace {
    Shader* activeShader = nullptr;

    std::vector<glm::mat4> matrixStack = { glm::mat4(1.0f) };

    glm::vec3 curColor(1.0f, 1.0f, 1.0f);
    float curShininess = 32.0f;
    float curSpecularStrength = 0.35f;
    glm::vec3 curEmissive(0.0f, 0.0f, 0.0f);
    float curAlpha = 1.0f;
    bool curUnlit = false;
    const Texture* curTexture = nullptr;
    glm::vec2 curTexRepeat(1.0f, 1.0f);

    SceneLighting activeLighting;

    // Immediate-mode accumulation state
    unsigned int immMode = GL_TRIANGLES;
    std::vector<Vertex> immVerts;
    glm::vec3 immCurNormal(0.0f, 0.0f, 1.0f);
    glm::vec3 immCurColor(1.0f, 1.0f, 1.0f);
    glm::vec2 immCurTexCoord(0.0f, 0.0f);
    glm::mat3 immNormalMatrix(1.0f);

    // Shape inspector state
    GLCompat::InspectSettings inspect;
    int counts[GLCompat::SHAPE_KIND_COUNT] = {0};
    bool haveBounds = false;
    glm::vec3 boundsMin(0.0f), boundsMax(0.0f);

    void addToBounds(const glm::vec3& p) {
        if (!haveBounds) {
            boundsMin = boundsMax = p;
            haveBounds = true;
        } else {
            boundsMin = glm::min(boundsMin, p);
            boundsMax = glm::max(boundsMax, p);
        }
    }

    // Offset that pushes a part away from the inspected object's center ("exploded view").
    glm::vec3 explodeOffset(const glm::vec3& partCenter) {
        if (inspect.explode <= 0.0f) return glm::vec3(0.0f);
        return (partCenter - inspect.explodeCenter) * inspect.explode;
    }

    // Uploads every material uniform for the next draw call.
    void applyMaterial(GLCompat::ShapeKind kind) {
        Shader& s = *activeShader;
        glm::vec3 color = curColor;
        glm::vec3 emissive = curEmissive;
        float alpha = curAlpha;
        bool unlit = curUnlit;
        float shininess = curShininess;
        float spec = curSpecularStrength;
        bool textured = (curTexture != nullptr && curTexture->IsLoaded());

        if (inspect.forceColor) {
            color = inspect.forcedColor;
            emissive = glm::vec3(0.0f);
            alpha = 1.0f;
            unlit = true;
            textured = false;
        } else if (inspect.colorByShape) {
            // Hue = shape kind; a small brightness step per part keeps neighbouring
            // parts of the same kind (e.g. many boxes) visibly separate.
            static const float steps[6] = {1.00f, 0.80f, 1.12f, 0.90f, 1.05f, 0.74f};
            int partIndex = 0;
            for (int k = 0; k < GLCompat::SHAPE_KIND_COUNT; ++k) partIndex += counts[k];
            color = glm::clamp(GLCompat::shapeColor(kind) * steps[partIndex % 6], 0.0f, 1.0f);
            emissive = glm::vec3(0.0f);
            alpha = 1.0f;
            unlit = false;
            shininess = 48.0f;
            spec = 0.35f;
            textured = false;
        }

        s.SetVec3("materialColor", color);
        s.SetFloat("shininess", shininess);
        s.SetFloat("materialSpecularStrength", spec);
        s.SetVec3("emissive", emissive);
        s.SetFloat("alphaValue", alpha);
        s.SetBool("unlit", unlit);
        s.SetBool("useTexture", textured);
        if (textured) {
            curTexture->Bind(0);
            s.SetInt("diffuseTexture", 0);
            s.SetVec2("texRepeat", curTexRepeat);
        }
    }

    // Common path for every cached primitive mesh. localMin/localMax is the part's
    // bounding box in its own (unscaled mesh) space - used for the exploded view and
    // for measuring the object's overall size.
    // Faint glow helpers (light halos, beams) are effects, not modelled parts.
    bool isGlowEffect() {
        return (inspect.colorByShape || inspect.measureOnly) && curAlpha < 0.2f;
    }

    void drawShape(GLCompat::ShapeKind kind, Mesh& mesh, const glm::mat4& model,
                   const glm::vec3& localMin, const glm::vec3& localMax) {
        if (!activeShader || isGlowEffect()) return;
        counts[kind]++;

        glm::vec3 localCenter = (localMin + localMax) * 0.5f;
        glm::vec3 worldCenter = glm::vec3(model * glm::vec4(localCenter, 1.0f));
        glm::mat4 finalModel = glm::translate(glm::mat4(1.0f), explodeOffset(worldCenter)) * model;

        for (int c = 0; c < 8; ++c) {
            glm::vec3 corner((c & 1) ? localMax.x : localMin.x,
                             (c & 2) ? localMax.y : localMin.y,
                             (c & 4) ? localMax.z : localMin.z);
            addToBounds(glm::vec3(finalModel * glm::vec4(corner, 1.0f)));
        }
        if (inspect.measureOnly) return;

        applyMaterial(kind);
        mesh.DrawGeometry(*activeShader, finalModel);
    }
}

// =====================================================================================
// FRAME SETUP & LIGHTING
// =====================================================================================
void GLCompat::beginFrame(Shader& shader, Camera& camera, const SceneLighting& lighting) {
    activeShader = &shader;
    activeLighting = lighting;
    matrixStack.clear();
    matrixStack.push_back(glm::mat4(1.0f));
    curAlpha = 1.0f;
    curUnlit = false;
    curTexture = nullptr;

    camera.Apply(shader);
    shader.Activate();

    // 1. Directional Light
    shader.SetBool("dirLightOn", lighting.dir.enabled);
    shader.SetVec3("dirLightDir", lighting.dir.direction);
    shader.SetVec3("dirLightColor", lighting.dir.color);
    shader.SetFloat("dirAmbientIntensity", lighting.dir.ambientIntensity);
    shader.SetFloat("dirDiffuseIntensity", lighting.dir.diffuseIntensity);
    shader.SetFloat("dirSpecularIntensity", lighting.dir.specularIntensity);
    shader.SetVec3("ambientSky", lighting.ambientSky);
    shader.SetVec3("ambientGround", lighting.ambientGround);

    // 2. Point Lights: slot 0 is the main point light, the rest are fill lamps.
    // All of them follow the main point light's on/off switch.
    int numPoints = 0;
    if (lighting.point.enabled) {
        const PointLightData* lights[1 + MAX_FILL_LIGHTS];
        lights[numPoints++] = &lighting.point;
        for (int i = 0; i < lighting.numFill && i < MAX_FILL_LIGHTS; ++i) {
            lights[numPoints++] = &lighting.fill[i];
        }
        for (int i = 0; i < numPoints; ++i) {
            std::string base = "pointLights[" + std::to_string(i) + "].";
            shader.SetVec3((base + "position").c_str(), lights[i]->position);
            shader.SetVec3((base + "color").c_str(), lights[i]->color);
            shader.SetFloat((base + "constant").c_str(), lights[i]->constant);
            shader.SetFloat((base + "linear").c_str(), lights[i]->linear);
            shader.SetFloat((base + "quadratic").c_str(), lights[i]->quadratic);
            shader.SetFloat((base + "intensity").c_str(), lights[i]->intensity);
        }
    }
    shader.SetInt("numPointLights", numPoints);

    // 3. Spot Light
    shader.SetBool("spotLightOn", lighting.spot.enabled);
    shader.SetVec3("spotLightPos", lighting.spot.position);
    shader.SetVec3("spotLightDir", lighting.spot.direction);
    shader.SetVec3("spotLightColor", lighting.spot.color);
    shader.SetFloat("spotLightCutOff", lighting.spot.cutOff);
    shader.SetFloat("spotLightOuterCutOff", lighting.spot.outerCutOff);
    shader.SetFloat("spotLightIntensity", lighting.spot.intensity);

    // Distance haze
    shader.SetBool("fogOn", lighting.fogEnabled);
    shader.SetVec3("fogColor", lighting.fogColor);
    shader.SetFloat("fogDensity", lighting.fogDensity);
}

const SceneLighting& GLCompat::getLighting() {
    return activeLighting;
}

// =====================================================================================
// MATRIX STACK
// =====================================================================================
void GLCompat::pushMatrix() {
    matrixStack.push_back(matrixStack.back());
}

void GLCompat::popMatrix() {
    if (matrixStack.size() > 1) matrixStack.pop_back();
}

void GLCompat::translatef(float x, float y, float z) {
    matrixStack.back() = glm::translate(matrixStack.back(), glm::vec3(x, y, z));
}

void GLCompat::rotatef(float degrees, float x, float y, float z) {
    matrixStack.back() = glm::rotate(matrixStack.back(), glm::radians(degrees), glm::vec3(x, y, z));
}

void GLCompat::scalef(float x, float y, float z) {
    matrixStack.back() = glm::scale(matrixStack.back(), glm::vec3(x, y, z));
}

const glm::mat4& GLCompat::topMatrix() {
    return matrixStack.back();
}

// =====================================================================================
// MATERIAL, TEXTURE & BLEND STATE
// =====================================================================================
void GLCompat::setMaterial(float r, float g, float b, float shininess, float spec) {
    curColor = glm::vec3(r, g, b);
    curShininess = shininess;
    curSpecularStrength = spec;
}

void GLCompat::setEmissive(float r, float g, float b, float intensity) {
    curEmissive = glm::vec3(r * intensity, g * intensity, b * intensity);
}

void GLCompat::resetEmission() {
    curEmissive = glm::vec3(0.0f);
}

void GLCompat::setTexture(const Texture* texture, float uRepeat, float vRepeat) {
    curTexture = texture;
    curTexRepeat = glm::vec2(uRepeat, vRepeat);
}

void GLCompat::clearTexture() {
    curTexture = nullptr;
}

void GLCompat::setAlpha(float alpha) {
    curAlpha = alpha;
}

void GLCompat::setUnlit(bool unlit) {
    curUnlit = unlit;
}

void GLCompat::enableBlend() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GLCompat::enableAdditiveBlend() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

void GLCompat::disableBlend() {
    glDisable(GL_BLEND);
}

void GLCompat::setFog(bool enabled) {
    if (activeShader) activeShader->SetBool("fogOn", enabled && activeLighting.fogEnabled);
}

void GLCompat::setDepthWrite(bool enabled) {
    // In the shape inspector every part is drawn opaque, so it must also write depth.
    if (inspect.colorByShape) enabled = true;
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

// =====================================================================================
// PRIMITIVE DRAWS
// =====================================================================================
void GLCompat::drawBox(float sx, float sy, float sz) {
    glm::mat4 model = glm::scale(matrixStack.back(), glm::vec3(sx, sy, sz));
    drawShape(SHAPE_BOX, Primitives::unitBox(), model, glm::vec3(-0.5f), glm::vec3(0.5f));
}

void GLCompat::drawSphere(float radius, int, int) {
    glm::mat4 model = glm::scale(matrixStack.back(), glm::vec3(radius, radius, radius));
    drawShape(SHAPE_SPHERE, Primitives::unitSphere(), model, glm::vec3(-1.0f), glm::vec3(1.0f));
}

void GLCompat::drawCylinder(float baseRadius, float topRadius, float height, int slices) {
    float r = std::max(baseRadius, topRadius);
    drawShape(SHAPE_CYLINDER, Primitives::cylinder(baseRadius, topRadius, height, slices),
              matrixStack.back(), glm::vec3(-r, -r, 0.0f), glm::vec3(r, r, height));
}

void GLCompat::drawCone(float baseRadius, float height, int slices) {
    drawShape(SHAPE_CONE, Primitives::cylinder(baseRadius, 0.0f, height, slices),
              matrixStack.back(), glm::vec3(-baseRadius, -baseRadius, 0.0f),
              glm::vec3(baseRadius, baseRadius, height));
}

void GLCompat::drawDisk(float innerRadius, float outerRadius, int slices) {
    drawShape(SHAPE_DISK, Primitives::disk(innerRadius, outerRadius, slices),
              matrixStack.back(), glm::vec3(-outerRadius, -outerRadius, 0.0f),
              glm::vec3(outerRadius, outerRadius, 0.0f));
}

void GLCompat::drawTorus(float innerRadius, float outerRadius, int sides, int rings) {
    float r = innerRadius + outerRadius;
    drawShape(SHAPE_TORUS, Primitives::torus(innerRadius, outerRadius, sides, rings),
              matrixStack.back(), glm::vec3(-r, -r, -innerRadius), glm::vec3(r, r, innerRadius));
}

// =====================================================================================
// IMMEDIATE-MODE POLYGONS (glBegin / glEnd)
// =====================================================================================
void GLCompat::beginPrimitive(unsigned int mode) {
    immMode = mode;
    immVerts.clear();
    immCurNormal = glm::vec3(0.0f, 0.0f, 1.0f);
    immCurColor = curColor; // GL_COLOR_MATERIAL semantics: inherits the current material color
    immCurTexCoord = glm::vec2(0.0f);
    immNormalMatrix = glm::inverseTranspose(glm::mat3(matrixStack.back()));
}

void GLCompat::normal3f(float x, float y, float z) {
    immCurNormal = glm::vec3(x, y, z);
}

void GLCompat::color3f(float r, float g, float b) {
    curColor = glm::vec3(r, g, b);
    immCurColor = glm::vec3(r, g, b);
}

void GLCompat::texCoord2f(float u, float v) {
    immCurTexCoord = glm::vec2(u, v);
}

void GLCompat::vertex3f(float x, float y, float z) {
    glm::vec4 worldPos = matrixStack.back() * glm::vec4(x, y, z, 1.0f);
    glm::vec3 worldNormal = glm::normalize(immNormalMatrix * immCurNormal);
    immVerts.push_back({glm::vec3(worldPos), worldNormal, immCurColor, immCurTexCoord});
}

namespace {
    GLuint dynVAO = 0;
    GLuint dynVBO = 0;
    GLuint dynEBO = 0;
}

void GLCompat::endPrimitive() {
    std::vector<GLuint> idx;
    GLenum drawMode = GL_TRIANGLES;
    size_t n = immVerts.size();

    if (immMode == GL_QUADS) {
        for (size_t q = 0; q + 3 < n; q += 4) {
            idx.push_back((GLuint)q + 0); idx.push_back((GLuint)q + 1); idx.push_back((GLuint)q + 2);
            idx.push_back((GLuint)q + 0); idx.push_back((GLuint)q + 2); idx.push_back((GLuint)q + 3);
        }
    } else if (immMode == GL_TRIANGLE_FAN) {
        for (size_t i = 1; i + 1 < n; ++i) {
            idx.push_back(0); idx.push_back((GLuint)i); idx.push_back((GLuint)i + 1);
        }
    } else if (immMode == GL_LINE_LOOP) {
        for (size_t i = 0; i < n; ++i) idx.push_back((GLuint)i);
        drawMode = GL_LINE_LOOP;
    } else { // GL_TRIANGLES
        for (size_t i = 0; i + 2 < n; i += 3) {
            idx.push_back((GLuint)i); idx.push_back((GLuint)i + 1); idx.push_back((GLuint)i + 2);
        }
    }

    if (!idx.empty() && activeShader && !isGlowEffect()) {
        ShapeKind kind = (drawMode == GL_LINE_LOOP) ? SHAPE_LINE : SHAPE_POLYGON;
        counts[kind]++;

        // Exploded view: move the whole batch by the offset of its centroid.
        glm::vec3 centroid(0.0f);
        for (const Vertex& v : immVerts) centroid += v.position;
        centroid /= (float)n;
        glm::vec3 offset = explodeOffset(centroid);
        for (Vertex& v : immVerts) {
            v.position += offset;
            addToBounds(v.position);
        }

        if (!inspect.measureOnly) {
            if (dynVAO == 0) {
                glGenVertexArrays(1, &dynVAO);
                glGenBuffers(1, &dynVBO);
                glGenBuffers(1, &dynEBO);

                glBindVertexArray(dynVAO);
                glBindBuffer(GL_ARRAY_BUFFER, dynVBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dynEBO);

                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
                glEnableVertexAttribArray(3);
                glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
            }

            glBindVertexArray(dynVAO);
            glBindBuffer(GL_ARRAY_BUFFER, dynVBO);
            glBufferData(GL_ARRAY_BUFFER, immVerts.size() * sizeof(Vertex), immVerts.data(), GL_STREAM_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dynEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STREAM_DRAW);

            // Vertex colors already carry the material color (GL_COLOR_MATERIAL style), so
            // the material color is white - unless the inspector overrides it.
            glm::vec3 savedColor = curColor;
            curColor = glm::vec3(1.0f);
            if (inspect.colorByShape || inspect.forceColor) {
                for (Vertex& v : immVerts) v.color = glm::vec3(1.0f);
                glBufferData(GL_ARRAY_BUFFER, immVerts.size() * sizeof(Vertex), immVerts.data(), GL_STREAM_DRAW);
            }
            activeShader->Activate();
            activeShader->SetMat4("model", glm::mat4(1.0f));
            applyMaterial(kind);
            curColor = savedColor;

            glDrawElements(drawMode, (GLsizei)idx.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }
    immVerts.clear();
}

void GLCompat::drawGroundShadow(float x, float z, float radiusX, float radiusZ, float alpha) {
    // Blob shadows are a lighting trick, not a modelled part - hide them in the inspector.
    if (!activeShader || inspect.colorByShape || inspect.measureOnly || inspect.forceColor) return;
    bool savedUnlit = curUnlit;
    float savedAlpha = curAlpha;

    setUnlit(true);
    setAlpha(alpha);
    enableBlend();
    setDepthWrite(false);

    pushMatrix();
    translatef(x, 0.015f, z);
    scalef(radiusX, 1.0f, radiusZ);
    rotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Rotate unit disk flat onto ground
    Primitives::disk(0.0f, 1.0f, 16)
        .Draw(*activeShader, matrixStack.back(), glm::vec3(0.12f, 0.08f, 0.05f), 0.0f, glm::vec3(0.0f), alpha, true);
    popMatrix();

    setDepthWrite(true);
    disableBlend();
    setUnlit(savedUnlit);
    setAlpha(savedAlpha);
}

// =====================================================================================
// SHAPE INSPECTOR
// =====================================================================================
const char* GLCompat::shapeName(ShapeKind kind) {
    switch (kind) {
        case SHAPE_BOX:      return "Box (cube)";
        case SHAPE_SPHERE:   return "Sphere";
        case SHAPE_CYLINDER: return "Cylinder";
        case SHAPE_CONE:     return "Cone";
        case SHAPE_DISK:     return "Disk / ring";
        case SHAPE_TORUS:    return "Torus";
        case SHAPE_POLYGON:  return "Custom polygon";
        case SHAPE_LINE:     return "Line loop";
        default:             return "?";
    }
}

glm::vec3 GLCompat::shapeColor(ShapeKind kind) {
    switch (kind) {
        case SHAPE_BOX:      return glm::vec3(0.22f, 0.50f, 0.95f); // blue
        case SHAPE_SPHERE:   return glm::vec3(0.98f, 0.55f, 0.12f); // orange
        case SHAPE_CYLINDER: return glm::vec3(0.18f, 0.74f, 0.36f); // green
        case SHAPE_CONE:     return glm::vec3(0.96f, 0.82f, 0.10f); // yellow
        case SHAPE_DISK:     return glm::vec3(0.62f, 0.34f, 0.90f); // purple
        case SHAPE_TORUS:    return glm::vec3(0.92f, 0.22f, 0.40f); // pink-red
        case SHAPE_POLYGON:  return glm::vec3(0.10f, 0.74f, 0.80f); // teal
        case SHAPE_LINE:     return glm::vec3(0.20f, 0.20f, 0.22f); // dark grey
        default:             return glm::vec3(0.5f);
    }
}

void GLCompat::setInspect(const InspectSettings& settings) {
    inspect = settings;
}

const GLCompat::InspectSettings& GLCompat::getInspect() {
    return inspect;
}

void GLCompat::resetStats() {
    for (int i = 0; i < SHAPE_KIND_COUNT; ++i) counts[i] = 0;
    haveBounds = false;
}

int GLCompat::shapeCount(ShapeKind kind) {
    return counts[kind];
}

bool GLCompat::statsBounds(glm::vec3& minCorner, glm::vec3& maxCorner) {
    minCorner = boundsMin;
    maxCorner = boundsMax;
    return haveBounds;
}
