#ifndef ENGINE_GLCOMPAT_H
#define ENGINE_GLCOMPAT_H

#include "engine/Shader.h"
#include "engine/Camera.h"
#include "core/Types.h"
#include <glm/glm.hpp>

// Authoring-time compatibility layer used to port the ~2500 lines of already-tuned scene
// geometry (Doll/Guards/Contestants/Environment) onto the new shader-based engine with
// minimal textual changes. Under the hood it is 100% modern OpenGL - real shaders, real
// VAO/VBO/EBO meshes (see Primitives.h), real GLM matrices - only the call-site *shape*
// (pushMatrix/translatef/rotatef/scalef, drawSphere/drawBox/..., setMaterial/setEmissive)
// deliberately rhymes with the legacy fixed-function calls so that code doesn't need every
// one of its hundreds of transform/primitive calls hand-retyped.
namespace GLCompat {
    // Call once per frame with complete 3-type lighting and shading configuration:
    void beginFrame(Shader& shader, Camera& camera, const SceneLighting& lighting);

    // Call once per frame before drawing the scene: binds the shader and uploads the
    // camera + directional light uniforms every draw call will share.
    void beginFrame(Shader& shader, Camera& camera, const glm::vec3& lightDir, const glm::vec3& lightColor);

    // Dynamic Lighting & Shading mode accessors
    const SceneLighting& getLighting();
    void setLighting(const SceneLighting& lighting);
    void toggleGouraudShading();
    bool isGouraudShading();

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

    // Translucency state (mirrors the legacy glPushAttrib/glEnable(GL_BLEND)/
    // glDisable(GL_LIGHTING)/glColor4f pattern used for blush, laser-scan cones, and the
    // ground shadow). setAlpha/setUnlit affect every draw call below until reset back to
    // (1.0, false); enableBlend/disableBlend and setDepthWrite toggle the actual GL state.
    void setAlpha(float alpha);
    void setUnlit(bool unlit);
    void enableBlend();
    void disableBlend();
    void setDepthWrite(bool enabled);

    // Primitive draws at the current top-of-stack transform with the current material
    // (mirrors Renderer::drawBox/drawCylinder/drawCone/drawSphere/drawDisk/drawTorus).
    void drawBox(float sx, float sy, float sz);
    void drawSphere(float radius, int slices = 16, int stacks = 16);
    void drawCylinder(float baseRadius, float topRadius, float height, int slices = 16);
    void drawCone(float baseRadius, float height, int slices = 16);
    void drawDisk(float innerRadius, float outerRadius, int slices = 20);
    void drawTorus(float innerRadius, float outerRadius, int sides = 12, int rings = 16);

    // Immediate-mode vertex accumulation (mirrors glBegin/glNormal3f/glColor3f/glVertex3f/
    // glEnd). Supports GL_TRIANGLES, GL_QUADS, GL_TRIANGLE_FAN and GL_LINE_LOOP - the modes
    // actually used by the legacy scene code (custom-triangulated meshes like the pleated
    // skirt and sky walls, and thin line-art like the house/guard hut symbols). Vertices are
    // transformed by the current top-of-stack matrix immediately, exactly like real
    // immediate-mode GL did, so the accumulated mesh can be drawn with an identity model.
    // beginPrimitive() seeds the per-vertex color from the current material color (set via
    // setMaterial), matching GL_COLOR_MATERIAL's real behavior; color3f() overrides it for
    // subsequent vertices only, e.g. for a hand-authored color gradient.
    void beginPrimitive(unsigned int mode);
    void normal3f(float x, float y, float z);
    void color3f(float r, float g, float b);
    void vertex3f(float x, float y, float z);
    void endPrimitive();

    // Flat, translucent blob shadow under a character (mirrors Renderer::drawGroundShadow).
    void drawGroundShadow(float x, float z, float radiusX, float radiusZ, float alpha = 0.35f);
}

#endif // ENGINE_GLCOMPAT_H
