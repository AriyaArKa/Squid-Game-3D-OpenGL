#include "engine/GLCompat.h"
#include "engine/Primitives.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace {
    Shader* activeShader = nullptr;

    std::vector<glm::mat4> matrixStack = { glm::mat4(1.0f) };

    glm::vec3 curColor(1.0f, 1.0f, 1.0f);
    float curShininess = 32.0f;
    glm::vec3 curEmissive(0.0f, 0.0f, 0.0f);
}

void GLCompat::beginFrame(Shader& shader, Camera& camera, const glm::vec3& lightDir, const glm::vec3& lightColor) {
    activeShader = &shader;
    matrixStack.clear();
    matrixStack.push_back(glm::mat4(1.0f));

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

void GLCompat::drawBox(float sx, float sy, float sz) {
    glm::mat4 model = glm::scale(matrixStack.back(), glm::vec3(sx, sy, sz));
    Primitives::unitBox().Draw(*activeShader, model, curColor, curShininess, curEmissive);
}

void GLCompat::drawSphere(float radius, int, int) {
    glm::mat4 model = glm::scale(matrixStack.back(), glm::vec3(radius, radius, radius));
    Primitives::unitSphere().Draw(*activeShader, model, curColor, curShininess, curEmissive);
}

void GLCompat::drawCylinder(float baseRadius, float topRadius, float height, int slices) {
    Primitives::cylinder(baseRadius, topRadius, height, slices)
        .Draw(*activeShader, matrixStack.back(), curColor, curShininess, curEmissive);
}

void GLCompat::drawCone(float baseRadius, float height, int slices) {
    Primitives::cylinder(baseRadius, 0.0f, height, slices)
        .Draw(*activeShader, matrixStack.back(), curColor, curShininess, curEmissive);
}

void GLCompat::drawDisk(float innerRadius, float outerRadius, int slices) {
    Primitives::disk(innerRadius, outerRadius, slices)
        .Draw(*activeShader, matrixStack.back(), curColor, curShininess, curEmissive);
}

void GLCompat::drawTorus(float innerRadius, float outerRadius, int sides, int rings) {
    Primitives::torus(innerRadius, outerRadius, sides, rings)
        .Draw(*activeShader, matrixStack.back(), curColor, curShininess, curEmissive);
}
