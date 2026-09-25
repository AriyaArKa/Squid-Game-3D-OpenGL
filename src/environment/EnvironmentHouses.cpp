#include "environment/Environment.h"
#include "render/Renderer.h"
#include "core/Config.h"
#include <GL/glut.h>

// Korean village houses that line the player's starting background (Reference Images 1, 2, 4).
// Split out of Environment.cpp because the house model is a self-contained "prop" you can
// read, tweak, or debug without touching the arena shell (ground/walls/lines) at all.

namespace {
    // Draw the Squid Game Geometric Symbols (○ △ □) on the house transom panel (Reference Image 4)
    void drawHouseSymbols(float w, float h) {
        Renderer::setMaterial(0.92f, 0.35f, 0.55f, 30.0f); // Hot pink symbol color

        // Circle ○
        glPushMatrix();
        glTranslatef(-w * 0.30f, 0.0f, 0.015f);
        Renderer::drawTorus(0.015f, 0.07f, 8, 16);
        glPopMatrix();

        // Triangle △
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.015f);
        glBegin(GL_LINE_LOOP);
        glLineWidth(2.5f);
        glVertex3f( 0.0f,   0.08f, 0.0f);
        glVertex3f(-0.07f, -0.06f, 0.0f);
        glVertex3f( 0.07f, -0.06f, 0.0f);
        glEnd();
        glPopMatrix();

        // Square □
        glPushMatrix();
        glTranslatef(w * 0.30f, 0.0f, 0.015f);
        glBegin(GL_LINE_LOOP);
        glLineWidth(2.5f);
        glVertex3f(-0.07f,  0.07f, 0.0f);
        glVertex3f( 0.07f,  0.07f, 0.0f);
        glVertex3f( 0.07f, -0.07f, 0.0f);
        glVertex3f(-0.07f, -0.07f, 0.0f);
        glEnd();
        glPopMatrix();
    }

    // Draw an authentic Korean Village House as seen in Reference Images 1, 2, 4
    void drawAuthenticVillageHouse(float x, float z, float rotY) {
        glPushMatrix();
        glTranslatef(x, 0.0f, z);
        glRotatef(rotY, 0.0f, 1.0f, 0.0f);

        // 1. White / Pale-Blue Stucco Building Body
        Renderer::setMaterial(0.88f, 0.90f, 0.93f, 15.0f);
        glPushMatrix();
        glTranslatef(0.0f, 1.6f, 0.0f);
        Renderer::drawBox(4.4f, 3.2f, 3.2f);
        glPopMatrix();

        // 2. Korean Traditional Orange / Terracotta Tiled Roof with Overhang
        Renderer::setMaterial(0.88f, 0.40f, 0.14f, 30.0f);
        glPushMatrix();
        glTranslatef(0.0f, 3.6f, 0.0f);

        // Sloped Hip/Gable Roof
        glBegin(GL_TRIANGLES);
        // Left gable
        glNormal3f(-0.707f, 0.707f, 0.0f);
        glVertex3f(-2.45f, -0.4f, -1.85f);
        glVertex3f(-2.45f, -0.4f,  1.85f);
        glVertex3f(-2.45f,  1.0f,  0.0f);
        // Right gable
        glNormal3f(0.707f, 0.707f, 0.0f);
        glVertex3f(2.45f, -0.4f,  1.85f);
        glVertex3f(2.45f, -0.4f, -1.85f);
        glVertex3f(2.45f,  1.0f,  0.0f);
        glEnd();

        // Front & Back slopes
        glBegin(GL_QUADS);
        // Front slope (facing entrance)
        glNormal3f(0.0f, 0.707f, 0.707f);
        glVertex3f(-2.5f, -0.4f,  1.85f);
        glVertex3f( 2.5f, -0.4f,  1.85f);
        glVertex3f( 2.5f,  1.0f,  0.0f);
        glVertex3f(-2.5f,  1.0f,  0.0f);
        // Back slope
        glNormal3f(0.0f, 0.707f, -0.707f);
        glVertex3f( 2.5f, -0.4f, -1.85f);
        glVertex3f(-2.5f, -0.4f, -1.85f);
        glVertex3f(-2.5f,  1.0f,  0.0f);
        glVertex3f( 2.5f,  1.0f,  0.0f);
        glEnd();

        // White Ridge Tile Crest atop the roof
        Renderer::setMaterial(0.92f, 0.92f, 0.92f, 20.0f);
        glPushMatrix();
        glTranslatef(0.0f, 1.05f, 0.0f);
        Renderer::drawBox(5.0f, 0.14f, 0.18f);
        glPopMatrix();

        glPopMatrix(); // End Roof

        // 3. Rectangular Chimney Stack (Images 1, 2, 4)
        Renderer::setMaterial(0.82f, 0.84f, 0.86f, 15.0f);
        glPushMatrix();
        glTranslatef(1.3f, 4.4f, 0.2f);
        Renderer::drawBox(0.65f, 1.4f, 0.65f);
        // Chimney Cap
        glTranslatef(0.0f, 0.75f, 0.0f);
        Renderer::drawBox(0.80f, 0.12f, 0.80f);
        glPopMatrix();

        // 4. Green Metal Security Double Gate Entrance (Front wall z = +1.61f)
        float frontZ = 1.62f;

        // Dark Green Door Outer Frame
        Renderer::setMaterial(0.10f, 0.38f, 0.24f, 35.0f);
        glPushMatrix();
        glTranslatef(-0.55f, 1.35f, frontZ);
        Renderer::drawBox(1.95f, 2.45f, 0.08f);
        glPopMatrix();

        // Door Paneling & Vertical Security Slats (Reference Image 1)
        Renderer::setMaterial(0.12f, 0.48f, 0.30f, 45.0f);
        for (float sx = -1.35f; sx <= 0.25f; sx += 0.14f) {
            glPushMatrix();
            glTranslatef(sx, 1.25f, frontZ + 0.04f);
            Renderer::drawBox(0.04f, 2.0f, 0.03f);
            glPopMatrix();
        }

        // Transom Header Panel above the Gate with ○ △ □ Symbols (Image 4!)
        Renderer::setMaterial(0.08f, 0.28f, 0.18f, 40.0f);
        glPushMatrix();
        glTranslatef(-0.55f, 2.35f, frontZ + 0.04f);
        Renderer::drawBox(1.85f, 0.35f, 0.04f);
        // Symbols
        drawHouseSymbols(1.8f, 0.3f);
        glPopMatrix();

        // Twin Brass / Iron Door Rings
        Renderer::setMaterial(0.85f, 0.75f, 0.25f, 60.0f);
        glPushMatrix();
        glTranslatef(-0.65f, 1.30f, frontZ + 0.06f);
        Renderer::drawTorus(0.015f, 0.05f, 8, 12);
        glTranslatef(0.20f, 0.0f, 0.0f);
        Renderer::drawTorus(0.015f, 0.05f, 8, 12);
        glPopMatrix();

        // 5. Arched Green Window with Panes on the Right Side (Reference Images 1, 4)
        Renderer::setMaterial(0.10f, 0.40f, 0.25f, 40.0f);
        glPushMatrix();
        glTranslatef(1.15f, 1.8f, frontZ + 0.02f);
        // Lower window box
        Renderer::drawBox(0.95f, 1.15f, 0.04f);
        // Arched top
        glPushMatrix();
        glTranslatef(0.0f, 0.58f, 0.0f);
        glScalef(0.48f, 0.38f, 0.04f);
        glutSolidSphere(1.0, 14, 14);
        glPopMatrix();

        // Window Mullions / Panes (Dark Green)
        Renderer::setMaterial(0.06f, 0.24f, 0.15f, 30.0f);
        // Vertical mullion
        glPushMatrix(); glTranslatef(0.0f, 0.08f, 0.03f); Renderer::drawBox(0.04f, 1.35f, 0.03f); glPopMatrix();
        // Horizontal mullion
        glPushMatrix(); glTranslatef(0.0f, 0.15f, 0.03f); Renderer::drawBox(0.90f, 0.04f, 0.03f); glPopMatrix();
        glPopMatrix();

        glPopMatrix();
    }
}

void EnvironmentManager::drawBackgroundHouses() {
    // =========================================================================
    // HOUSES ONLY AT PLAYER START BACKGROUND (Reference Images 1 & 2)
    // THERE ARE NO HOUSES AT THE DOLL SIDE! (as explicitly requested)
    // =========================================================================

    // 1. Row of 5 Authentic Village Houses lined up behind all 456 contestants (Image 2!)
    // Back wall is at z = -ARENA_HALF_LENGTH (-75). Houses stand at z = -68.0f facing forward (+Z).
    drawAuthenticVillageHouse(-17.5f, -67.0f, 0.0f);
    drawAuthenticVillageHouse( -8.8f, -67.0f, 0.0f);
    drawAuthenticVillageHouse(  0.0f, -67.0f, 0.0f);
    drawAuthenticVillageHouse(  8.8f, -67.0f, 0.0f);
    drawAuthenticVillageHouse( 17.5f, -67.0f, 0.0f);

    // 2. Side Houses along Left & Right Walls near the start line (Reference Image 1!)
    // Left Wall houses facing into the arena (+X, rotY = 90)
    drawAuthenticVillageHouse(-ARENA_HALF_WIDTH + 2.0f, -56.0f, 90.0f);
    drawAuthenticVillageHouse(-ARENA_HALF_WIDTH + 2.0f, -44.0f, 90.0f);

    // Right Wall houses facing into the arena (-X, rotY = -90)
    drawAuthenticVillageHouse( ARENA_HALF_WIDTH - 2.0f, -56.0f, -90.0f);
    drawAuthenticVillageHouse( ARENA_HALF_WIDTH - 2.0f, -44.0f, -90.0f);
}
