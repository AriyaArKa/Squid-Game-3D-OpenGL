#pragma once

#include <glad/gl.h>
#include <cmath>
#include "engine/GLCompat.h"

// GLUT 3D primitives implemented using modern GLCompat engine
inline void glutSolidCube(double size) {
    GLCompat::drawBox((float)size, (float)size, (float)size);
}

inline void glutSolidSphere(double radius, int slices, int stacks) {
    GLCompat::drawSphere((float)radius, slices, stacks);
}

inline void glutSolidTorus(double innerRadius, double outerRadius, int sides, int rings) {
    GLCompat::drawTorus((float)innerRadius, (float)outerRadius, sides, rings);
}

// Matrix stack operations routed to GLCompat's glm::mat4 stack
inline void glPushMatrix() {
    GLCompat::pushMatrix();
}

inline void glPopMatrix() {
    GLCompat::popMatrix();
}

inline void glTranslatef(float x, float y, float z) {
    GLCompat::translatef(x, y, z);
}

inline void glRotatef(float angleDeg, float x, float y, float z) {
    GLCompat::rotatef(angleDeg, x, y, z);
}

inline void glScalef(float x, float y, float z) {
    GLCompat::scalef(x, y, z);
}

// Immediate-mode primitives accumulated and batched into modern VBO/VAO meshes
inline void glBegin(unsigned int mode) {
    GLCompat::beginPrimitive(mode);
}

inline void glEnd() {
    GLCompat::endPrimitive();
}

inline void glVertex3f(float x, float y, float z) {
    GLCompat::vertex3f(x, y, z);
}

inline void glNormal3f(float x, float y, float z) {
    GLCompat::normal3f(x, y, z);
}

inline void glTexCoord2f(float u, float v) {
    GLCompat::texCoord2f(u, v);
}

inline void glColor3f(float r, float g, float b) {
    GLCompat::color3f(r, g, b);
}

inline void glColor4f(float r, float g, float b, float a) {
    GLCompat::color3f(r, g, b);
    GLCompat::setAlpha(a);
}

// GL state constants & attribute stack compatibility for legacy fixed-function calls
#ifndef GL_LIGHTING
#define GL_LIGHTING 0x0B50
#endif
#ifndef GL_ENABLE_BIT
#define GL_ENABLE_BIT 0x00002000
#endif
#ifndef GL_COLOR_BUFFER_BIT
#define GL_COLOR_BUFFER_BIT 0x00004000
#endif
#ifndef GL_DEPTH_BUFFER_BIT
#define GL_DEPTH_BUFFER_BIT 0x00000100
#endif

#ifndef GLUT_LEFT_BUTTON
#define GLUT_LEFT_BUTTON 0
#endif
#ifndef GLUT_MIDDLE_BUTTON
#define GLUT_MIDDLE_BUTTON 1
#endif
#ifndef GLUT_RIGHT_BUTTON
#define GLUT_RIGHT_BUTTON 2
#endif
#ifndef GLUT_DOWN
#define GLUT_DOWN 0
#endif
#ifndef GLUT_UP
#define GLUT_UP 1
#endif

inline void glPushAttrib(unsigned int) {}
inline void glPopAttrib() {
    GLCompat::setUnlit(false);
    GLCompat::setAlpha(1.0f);
    GLCompat::disableBlend();
    GLCompat::setDepthWrite(true);
}

inline void gluLookAt(double, double, double, double, double, double, double, double, double) {}
