#ifndef ENGINE_GLCOMPAT_H
#define ENGINE_GLCOMPAT_H

#include "engine/Shader.h"
#include "engine/Camera.h"
#include <glm/glm.hpp>

// Authoring-time compatibility layer used to port the ~2500 lines of already-tuned scene
// geometry (Doll/Guards/Contestants/Environment) onto the new shader-based engine with
// minimal textual changes. Under the hood it is 100% modern OpenGL - real shaders, real
// VAO/VBO/EBO meshes (see Primitives.h), real GLM matrices - only the call-site *shape*
// (pushMatrix/translatef/rotatef/scalef, drawSphere/drawBox/..., setMaterial/setEmissive)
// deliberately rhymes with the legacy fixed-function calls so that code doesn't need every
// one of its hundreds of transform/primitive calls hand-retyped.
namespace GLCompat {
    // Call once per frame before drawing the scene: binds the shader and uploads the
    // camera + directional light uniforms every draw call will share.
    void beginFrame(Shader& shader, Camera& camera, const glm::vec3& lightDir, const glm::vec3& lightColor);

    // Matrix stack (mirrors glPushMatrix/glPopMatrix/glTranslatef/glRotatef/glScalef).
    void pushMatrix();
    void popMatrix();
    void translatef(float x, float y, float z);
    void rotatef(float degrees, float x, float y, float z);
    void scalef(float x, float y, float z);
    const glm::mat4& topMatrix();

    // Material state (mirrors Renderer::setMaterial/setEmissive/resetEmission).
    void setMaterial(float r, float g, float b, float shininess = 32.0f, float spec = 0.35f);
    void setEmissive(float r, float g, float b, float intensity = 1.0f);
    void resetEmission();

    // Primitive draws at the current top-of-stack transform with the current material
    // (mirrors Renderer::drawBox/drawCylinder/drawCone/drawSphere/drawDisk/drawTorus).
    void drawBox(float sx, float sy, float sz);
    void drawSphere(float radius, int slices = 16, int stacks = 16);
    void drawCylinder(float baseRadius, float topRadius, float height, int slices = 16);
    void drawCone(float baseRadius, float height, int slices = 16);
    void drawDisk(float innerRadius, float outerRadius, int slices = 20);
    void drawTorus(float innerRadius, float outerRadius, int sides = 12, int rings = 16);
}

#endif // ENGINE_GLCOMPAT_H
