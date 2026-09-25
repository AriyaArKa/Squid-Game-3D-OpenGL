#include "render/Renderer.h"
#include <cmath>

namespace {
    bool smoothMode = true;
    GLUquadric* quadric = nullptr;
}

void Renderer::initGL() {
    quadric = gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_FALSE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.55f, 0.78f, 0.95f, 1.0f); // Warm atmospheric sky color
}

void Renderer::setupLighting() {
    // Warm sun light
    GLfloat lightPos[4] = { 15.0f, 30.0f, 10.0f, 0.0f }; // Directional sunlight
    GLfloat ambientLight[4]  = { 0.45f, 0.45f, 0.48f, 1.0f };
    GLfloat diffuseLight[4]  = { 0.95f, 0.92f, 0.85f, 1.0f }; // Warm direct sun
    GLfloat specularLight[4] = { 0.70f, 0.70f, 0.70f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
}

void Renderer::toggleShading() {
    smoothMode = !smoothMode;
    glShadeModel(smoothMode ? GL_SMOOTH : GL_FLAT);
    if (quadric) {
        gluQuadricNormals(quadric, smoothMode ? GLU_SMOOTH : GLU_FLAT);
    }
}

bool Renderer::isSmoothShading() {
    return smoothMode;
}

void Renderer::setMaterial(float r, float g, float b, float shininess, float spec) {
    const float ka = 0.35f, kd = 0.85f;
    GLfloat ambient[4]  = { r * ka, g * ka, b * ka, 1.0f };
    GLfloat diffuse[4]  = { r * kd, g * kd, b * kd, 1.0f };
    GLfloat specular[4] = { spec, spec, spec, 1.0f };
    GLfloat emission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    glColor3f(r, g, b);
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

void Renderer::setEmissive(float r, float g, float b, float intensity) {
    GLfloat emission[4] = { r * intensity, g * intensity, b * intensity, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

void Renderer::resetEmission() {
    GLfloat emission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
}

void Renderer::drawBox(float sx, float sy, float sz) {
    glPushMatrix();
    glScalef(sx, sy, sz);
    glutSolidCube(1.0);
    glPopMatrix();
}

void Renderer::drawCylinder(float baseRadius, float topRadius, float height, int slices) {
    if (!quadric) quadric = gluNewQuadric();
    gluCylinder(quadric, baseRadius, topRadius, height, slices, 4);
}

void Renderer::drawCone(float baseRadius, float height, int slices) {
    if (!quadric) quadric = gluNewQuadric();
    gluCylinder(quadric, baseRadius, 0.0, height, slices, 4);
}

void Renderer::drawSphere(float radius, int slices, int stacks) {
    if (!quadric) quadric = gluNewQuadric();
    gluSphere(quadric, radius, slices, stacks);
}

void Renderer::drawDisk(float innerRadius, float outerRadius, int slices) {
    if (!quadric) quadric = gluNewQuadric();
    gluDisk(quadric, innerRadius, outerRadius, slices, 1);
}

void Renderer::drawTorus(float innerRadius, float outerRadius, int sides, int rings) {
    glutSolidTorus(innerRadius, outerRadius, sides, rings);
}

void Renderer::drawGroundShadow(float x, float z, float radiusX, float radiusZ, float alpha) {
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPushMatrix();
    glTranslatef(x, 0.015f, z);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

    glColor4f(0.12f, 0.08f, 0.05f, alpha);
    const int segments = 20;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * 3.14159265f * (float)i / (float)segments;
        glVertex2f(cosf(theta) * radiusX, sinf(theta) * radiusZ);
    }
    glEnd();

    glPopMatrix();
    glPopAttrib();
}
