#include "render/Renderer.h"
#include <cmath>
#include "engine/GLCompat.h"

namespace {
    bool smoothMode = true;
}

void Renderer::initGL() {}
void Renderer::setupLighting() {}

void Renderer::toggleShading() {
    smoothMode = !smoothMode;
}

bool Renderer::isSmoothShading() {
    return smoothMode;
}

void Renderer::setMaterial(float r, float g, float b, float shininess, float spec) {
    GLCompat::setMaterial(r, g, b, shininess, spec);
}

void Renderer::setEmissive(float r, float g, float b, float intensity) {
    GLCompat::setEmissive(r, g, b, intensity);
}

void Renderer::resetEmission() {
    GLCompat::resetEmission();
}

void Renderer::drawBox(float sx, float sy, float sz) {
    GLCompat::drawBox(sx, sy, sz);
}

void Renderer::drawCylinder(float baseRadius, float topRadius, float height, int slices) {
    GLCompat::drawCylinder(baseRadius, topRadius, height, slices);
}

void Renderer::drawCone(float baseRadius, float height, int slices) {
    GLCompat::drawCone(baseRadius, height, slices);
}

void Renderer::drawSphere(float radius, int slices, int stacks) {
    GLCompat::drawSphere(radius, slices, stacks);
}

void Renderer::drawDisk(float innerRadius, float outerRadius, int slices) {
    GLCompat::drawDisk(innerRadius, outerRadius, slices);
}

void Renderer::drawTorus(float innerRadius, float outerRadius, int sides, int rings) {
    GLCompat::drawTorus(innerRadius, outerRadius, sides, rings);
}

void Renderer::drawGroundShadow(float x, float z, float radiusX, float radiusZ, float alpha) {
    GLCompat::drawGroundShadow(x, z, radiusX, radiusZ, alpha);
}
