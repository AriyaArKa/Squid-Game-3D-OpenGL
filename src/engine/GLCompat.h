#ifndef ENGINE_GLCOMPAT_H
#define ENGINE_GLCOMPAT_H

#include "engine/Shader.h"
#include "engine/Camera.h"
#include "engine/Texture.h"
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
    // Call once per frame (per viewport): binds the shader and uploads the camera plus the
    // complete lighting setup (directional + point lights + spot light + ambient + fog).
    void beginFrame(Shader& shader, Camera& camera, const SceneLighting& lighting);

    const SceneLighting& getLighting();

    // Matrix stack (mirrors glPushMatrix/glPopMatrix/glTranslatef/glRotatef/glScalef).
    void pushMatrix();
    void popMatrix();
    void translatef(float x, float y, float z);
    void rotatef(float degrees, float x, float y, float z);
    void scalef(float x, float y, float z);
    const glm::mat4& topMatrix();

    // Material state (mirrors Renderer::setMaterial/setEmissive/resetEmission).
    void setMaterial(float r, float g, float b, float shininess = 32.0f, float spec = 0.35f);
    inline void setMaterial(const glm::vec3& color, float shininess = 32.0f, float spec = 0.35f) {
        setMaterial(color.r, color.g, color.b, shininess, spec);
    }
    void setEmissive(float r, float g, float b, float intensity = 1.0f);
    void resetEmission();

    // Texture state. While a texture is set, every draw samples it at
    // (u * uRepeat, v * vRepeat) and multiplies the result with the material color.
    // Pass nullptr (or call clearTexture) to go back to plain colored materials.
    void setTexture(const Texture* texture, float uRepeat = 1.0f, float vRepeat = 1.0f);
    void clearTexture();

    // Translucency state (mirrors the legacy glPushAttrib/glEnable(GL_BLEND)/
    // glDisable(GL_LIGHTING)/glColor4f pattern used for blush, laser-scan cones, and the
    // ground shadow). setAlpha/setUnlit affect every draw call below until reset back to
    // (1.0, false); enableBlend/disableBlend and setDepthWrite toggle the actual GL state.
    void setAlpha(float alpha);
    void setUnlit(bool unlit);
    void enableBlend();
    void enableAdditiveBlend();
    void disableBlend();
    void setDepthWrite(bool enabled);
    // Turns the frame's distance haze off/on for the following draws (e.g. the sky dome).
    void setFog(bool enabled);

    // Primitive draws at the current top-of-stack transform with the current material
    // (mirrors Renderer::drawBox/drawCylinder/drawCone/drawSphere/drawDisk/drawTorus).
    void drawBox(float sx, float sy, float sz);
    void drawSphere(float radius, int slices = 16, int stacks = 16);
    void drawCylinder(float baseRadius, float topRadius, float height, int slices = 16);
    void drawCone(float baseRadius, float height, int slices = 16);
    void drawDisk(float innerRadius, float outerRadius, int slices = 20);
    void drawTorus(float innerRadius, float outerRadius, int sides = 12, int rings = 16);

    // Immediate-mode vertex accumulation (mirrors glBegin/glNormal3f/glColor3f/glTexCoord2f/
    // glVertex3f/glEnd). Supports GL_TRIANGLES, GL_QUADS, GL_TRIANGLE_FAN and GL_LINE_LOOP -
    // the modes actually used by the scene code (custom-triangulated meshes like the pleated
    // skirt and sky walls, and thin line-art like the house symbols). Vertices are
    // transformed by the current top-of-stack matrix immediately, exactly like real
    // immediate-mode GL did, so the accumulated mesh can be drawn with an identity model.
    // beginPrimitive() seeds the per-vertex color from the current material color (set via
    // setMaterial), matching GL_COLOR_MATERIAL's real behavior; color3f() overrides it for
    // subsequent vertices only, e.g. for a hand-authored color gradient.
    void beginPrimitive(unsigned int mode);
    void normal3f(float x, float y, float z);
    void color3f(float r, float g, float b);
    void texCoord2f(float u, float v);
    void vertex3f(float x, float y, float z);
    void endPrimitive();

    // Flat, translucent blob shadow under a character (mirrors Renderer::drawGroundShadow).
    void drawGroundShadow(float x, float z, float radiusX, float radiusZ, float alpha = 0.35f);

    // ---------------------------------------------------------------------------------
    // SHAPE INSPECTOR (used by the in-game "Shape view" and the report figure generator)
    // ---------------------------------------------------------------------------------
    // Every draw call above belongs to one of these basic shape kinds.
    enum ShapeKind {
        SHAPE_BOX, SHAPE_SPHERE, SHAPE_CYLINDER, SHAPE_CONE,
        SHAPE_DISK, SHAPE_TORUS, SHAPE_POLYGON, SHAPE_LINE,
        SHAPE_KIND_COUNT
    };
    const char* shapeName(ShapeKind kind);
    glm::vec3 shapeColor(ShapeKind kind);

    struct InspectSettings {
        bool colorByShape = false;          // paint each part with its shape kind's color
        float explode = 0.0f;               // push parts apart: 0 = assembled, 0.5 = +50% spread
        glm::vec3 explodeCenter = glm::vec3(0.0f);
        bool measureOnly = false;           // count + measure bounds but draw nothing
        bool forceColor = false;            // flat unlit override color (wireframe overlays)
        glm::vec3 forcedColor = glm::vec3(0.0f);
    };
    void setInspect(const InspectSettings& settings);
    const InspectSettings& getInspect();

    // Statistics gathered since the last resetStats(): how many parts of each shape kind
    // were drawn, and the world-space bounding box that contains all of them.
    void resetStats();
    int shapeCount(ShapeKind kind);
    bool statsBounds(glm::vec3& minCorner, glm::vec3& maxCorner);
}

#endif // ENGINE_GLCOMPAT_H
