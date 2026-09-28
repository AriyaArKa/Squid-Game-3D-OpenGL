#include "engine/GLCompat.h"
#include "engine/Primitives.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <vector>
#include <memory>
#include <cmath>

namespace {
    Shader* activeShader = nullptr;

    std::vector<glm::mat4> matrixStack = { glm::mat4(1.0f) };

    glm::vec3 curColor(1.0f, 1.0f, 1.0f);
    float curShininess = 32.0f;
    glm::vec3 curEmissive(0.0f, 0.0f, 0.0f);
    float curAlpha = 1.0f;
    bool curUnlit = false;

    // Immediate-mode accumulation state
    unsigned int immMode = GL_TRIANGLES;
    std::vector<Vertex> immVerts;
    glm::vec3 immCurNormal(0.0f, 0.0f, 1.0f);
    glm::vec3 immCurColor(1.0f, 1.0f, 1.0f);
    glm::mat3 immNormalMatrix(1.0f);
}

void GLCompat::beginFrame(Shader& shader, Camera& camera, const glm::vec3& lightDir, const glm::vec3& lightColor) {
    activeShader = &shader;
    matrixStack.clear();
    matrixStack.push_back(glm::mat4(1.0f));
    curAlpha = 1.0f;
    curUnlit = false;

    camera.Apply(shader);
    shader.Activate();
    shader.SetVec3("lightDir", lightDir);
    shader.SetVec3("lightColor", lightColor);
}

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

void GLCompat::setMaterial(float r, float g, float b, float shininess, float /*spec*/) {
    curColor = glm::vec3(r, g, b);
    curShininess = shininess;
}

void GLCompat::setEmissive(float r, float g, float b, float intensity) {
    curEmissive = glm::vec3(r * intensity, g * intensity, b * intensity);
}

void GLCompat::resetEmission() {
    curEmissive = glm::vec3(0.0f);
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

void GLCompat::disableBlend() {
    glDisable(GL_BLEND);
}

void GLCompat::setDepthWrite(bool enabled) {
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void GLCompat::drawBox(float sx, float sy, float sz) {
    glm::mat4 model = glm::scale(matrixStack.back(), glm::vec3(sx, sy, sz));
    Primitives::unitBox().Draw(*activeShader, model, curColor, curShininess, curEmissive, curAlpha, curUnlit);
}

void GLCompat::drawSphere(float radius, int, int) {
    glm::mat4 model = glm::scale(matrixStack.back(), glm::vec3(radius, radius, radius));
    Primitives::unitSphere().Draw(*activeShader, model, curColor, curShininess, curEmissive, curAlpha, curUnlit);
}

void GLCompat::drawCylinder(float baseRadius, float topRadius, float height, int slices) {
    Primitives::cylinder(baseRadius, topRadius, height, slices)
        .Draw(*activeShader, matrixStack.back(), curColor, curShininess, curEmissive, curAlpha, curUnlit);
}

void GLCompat::drawCone(float baseRadius, float height, int slices) {
    Primitives::cylinder(baseRadius, 0.0f, height, slices)
        .Draw(*activeShader, matrixStack.back(), curColor, curShininess, curEmissive, curAlpha, curUnlit);
}

void GLCompat::drawDisk(float innerRadius, float outerRadius, int slices) {
    Primitives::disk(innerRadius, outerRadius, slices)
        .Draw(*activeShader, matrixStack.back(), curColor, curShininess, curEmissive, curAlpha, curUnlit);
}

void GLCompat::drawTorus(float innerRadius, float outerRadius, int sides, int rings) {
    Primitives::torus(innerRadius, outerRadius, sides, rings)
        .Draw(*activeShader, matrixStack.back(), curColor, curShininess, curEmissive, curAlpha, curUnlit);
}

void GLCompat::beginPrimitive(unsigned int mode) {
    immMode = mode;
    immVerts.clear();
    immCurNormal = glm::vec3(0.0f, 0.0f, 1.0f);
    immCurColor = curColor; // GL_COLOR_MATERIAL semantics: inherits the current material color
    immNormalMatrix = glm::inverseTranspose(glm::mat3(matrixStack.back()));
}

void GLCompat::normal3f(float x, float y, float z) {
    immCurNormal = glm::vec3(x, y, z);
}

void GLCompat::color3f(float r, float g, float b) {
    curColor = glm::vec3(r, g, b);
    immCurColor = glm::vec3(r, g, b);
}

void GLCompat::vertex3f(float x, float y, float z) {
    glm::vec4 worldPos = matrixStack.back() * glm::vec4(x, y, z, 1.0f);
    glm::vec3 worldNormal = glm::normalize(immNormalMatrix * immCurNormal);
    immVerts.push_back({glm::vec3(worldPos), worldNormal, immCurColor});
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
        drawMode = GL_TRIANGLES;
    } else if (immMode == GL_TRIANGLE_FAN) {
        for (size_t i = 1; i + 1 < n; ++i) {
            idx.push_back(0); idx.push_back((GLuint)i); idx.push_back((GLuint)i + 1);
        }
        drawMode = GL_TRIANGLES;
    } else if (immMode == GL_LINE_LOOP) {
        for (size_t i = 0; i < n; ++i) idx.push_back((GLuint)i);
        drawMode = GL_LINE_LOOP;
    } else { // GL_TRIANGLES
        for (size_t i = 0; i + 2 < n; i += 3) {
            idx.push_back((GLuint)i); idx.push_back((GLuint)i + 1); idx.push_back((GLuint)i + 2);
        }
        drawMode = GL_TRIANGLES;
    }

    if (!idx.empty() && activeShader) {
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
        }

        glBindVertexArray(dynVAO);
        glBindBuffer(GL_ARRAY_BUFFER, dynVBO);
        glBufferData(GL_ARRAY_BUFFER, immVerts.size() * sizeof(Vertex), immVerts.data(), GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dynEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STREAM_DRAW);

        activeShader->Activate();
        activeShader->SetMat4("model", glm::mat4(1.0f));
        activeShader->SetVec3("materialColor", glm::vec3(1.0f, 1.0f, 1.0f));
        activeShader->SetFloat("shininess", curShininess);
        activeShader->SetVec3("emissive", curEmissive);
        activeShader->SetFloat("alphaValue", curAlpha);
        activeShader->SetBool("unlit", curUnlit);

        glDrawElements(drawMode, (GLsizei)idx.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    immVerts.clear();
}

void GLCompat::drawGroundShadow(float x, float z, float radiusX, float radiusZ, float alpha) {
    if (!activeShader) return;
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
