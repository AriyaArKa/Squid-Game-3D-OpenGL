#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glut.h>
#include <glm/glm.hpp>

class Renderer {
public:
    static void initGL();
    static void setupLighting();

    // Material helpers
    static void setMaterial(float r, float g, float b, float shininess = 32.0f, float spec = 0.35f);
    static void setMaterial(const glm::vec3& color, float shininess = 32.0f, float spec = 0.35f);
    static void setEmissive(float r, float g, float b, float intensity = 1.0f);
    static void resetEmission();

    // Geometric primitives
    static void drawBox(float sx, float sy, float sz);
    static void drawCylinder(float baseRadius, float topRadius, float height, int slices = 16);
    static void drawCone(float baseRadius, float height, int slices = 16);
    static void drawSphere(float radius, int slices = 16, int stacks = 16);
    static void drawDisk(float innerRadius, float outerRadius, int slices = 20);
    static void drawTorus(float innerRadius, float outerRadius, int sides = 12, int rings = 16);
    static void drawGroundShadow(float x, float z, float radiusX, float radiusZ, float alpha = 0.35f);
};

#endif // RENDERER_H
