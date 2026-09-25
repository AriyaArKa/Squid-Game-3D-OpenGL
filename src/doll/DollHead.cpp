#include "doll/DollHead.h"
#include "render/Renderer.h"
#include <GL/glut.h>
#include <cmath>

namespace {
    // Spherical purple bead hair ties (matching the close-up screenshot!)
    void drawBeadHairTie(float x, float y, float z) {
        glPushMatrix();
        glTranslatef(x, y, z);
        // Deep vibrant Royal Purple beads matching the screenshot
        Renderer::setMaterial(0.56f, 0.20f, 0.82f, 65.0f);

        // 3-4 clustered beads around the base of the pigtail
        const float offsets[4][2] = {
            { 0.045f,  0.035f },
            {-0.045f,  0.030f },
            { 0.010f, -0.048f },
            {-0.020f, -0.010f }
        };
        for (int i = 0; i < 4; ++i) {
            glPushMatrix();
            glTranslatef(offsets[i][0], offsets[i][1], (i % 2 == 0 ? 0.02f : -0.02f));
            glutSolidSphere(0.046, 12, 12);
            glPopMatrix();
        }
        glPopMatrix();
    }

    // Sculpted Porcelain Ear (prominently featured in the iconic Episode 1 shot)
    void drawDollEar(bool isLeft) {
        float side = isLeft ? -1.0f : 1.0f;
        glPushMatrix();
        glTranslatef(side * 0.42f, -0.02f, 0.02f);
        glRotatef(side * 14.0f, 0.0f, 1.0f, 0.0f); // Slanted slightly outward
        glRotatef(-6.0f, 0.0f, 0.0f, 1.0f);

        // 1. Concha depression / Ear base (Ivory skin with soft cavity shading)
        Renderer::setMaterial(0.91f, 0.78f, 0.73f, 25.0f);
        glPushMatrix();
        glScalef(0.024f, 0.105f, 0.055f);
        glutSolidSphere(1.0, 14, 14);
        glPopMatrix();

        // 2. Outer Helix Rim (Curved outer edge of ear)
        Renderer::setMaterial(0.957f, 0.847f, 0.804f, 28.0f);
        glPushMatrix();
        glTranslatef(side * 0.012f, 0.022f, -0.012f);
        glScalef(0.016f, 0.088f, 0.022f);
        glutSolidSphere(1.0, 12, 12);
        glPopMatrix();

        // 3. Inner Antihelix & Tragus Cartilage
        glPushMatrix();
        glTranslatef(side * 0.008f, 0.008f, 0.012f);
        glScalef(0.014f, 0.055f, 0.016f);
        glutSolidSphere(1.0, 10, 10);
        glPopMatrix();

        // 4. Soft Earlobe at bottom
        glPushMatrix();
        glTranslatef(0.0f, -0.082f, -0.006f);
        glScalef(0.024f, 0.038f, 0.028f);
        glutSolidSphere(1.0, 12, 12);
        glPopMatrix();

        glPopMatrix();
    }

    void drawPigtail(bool isLeft) {
        float side = isLeft ? -1.0f : 1.0f;
        glPushMatrix();
        // Placed cleanly behind the ear and jawline
        glTranslatef(side * 0.44f, -0.07f, -0.10f);

        // Purple bead hair tie cluster right behind the ear base
        drawBeadHairTie(0.0f, 0.0f, 0.0f);

        // Glossy black hair strands
        Renderer::setMaterial(0.08f, 0.07f, 0.07f, 35.0f);

        // Upper pigtail segment
        glPushMatrix();
        glTranslatef(side * 0.05f, -0.16f, -0.02f);
        glRotatef(side * 20.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(12.0f, 1.0f, 0.0f, 0.0f);
        glScalef(0.14f, 0.26f, 0.14f);
        glutSolidSphere(1.0, 14, 14);
        glPopMatrix();

        // Lower curling tip
        glPushMatrix();
        glTranslatef(side * 0.12f, -0.36f, 0.0f);
        glRotatef(side * 32.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(16.0f, 1.0f, 0.0f, 0.0f);
        glScalef(0.11f, 0.20f, 0.11f);
        glutSolidSphere(1.0, 12, 12);
        glPopMatrix();

        glPopMatrix();
    }

    void drawEye(float x, float y, float z, bool isRedLight, float scanIntensity) {
        float side = (x < 0.0f) ? -1.0f : 1.0f;

        glPushMatrix();
        glTranslatef(x, y, z);

        // 1. Smooth White Sclera
        Renderer::setMaterial(0.97f, 0.97f, 0.97f, 30.0f);
        glPushMatrix();
        glScalef(0.078f, 0.072f, 0.030f);
        glutSolidSphere(1.0, 16, 16);
        glPopMatrix();

        // 2. Upper Eyelid & Eyelash Line (curved dark lash, winged outward at the outer corner)
        Renderer::setMaterial(0.04f, 0.04f, 0.04f, 20.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.052f, 0.022f);
        glScalef(0.082f, 0.014f, 0.016f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Winged eyeliner flick at the outer corner (reference close-up)
        glPushMatrix();
        glTranslatef(side * 0.076f, 0.060f, 0.016f);
        glRotatef(side * 28.0f, 0.0f, 0.0f, 1.0f);
        glScalef(0.028f, 0.010f, 0.013f);
        glutSolidCube(1.0);
        glPopMatrix();

        // 3. Iris (Deep glossy dark brown/black doll iris, large - nearly fills the eye as in reference)
        Renderer::setMaterial(0.10f, 0.08f, 0.07f, 80.0f);
        glPushMatrix();
        glTranslatef(0.0f, -0.004f, 0.024f);
        glScalef(0.062f, 0.060f, 0.017f);
        glutSolidSphere(1.0, 16, 16);
        glPopMatrix();

        // 4. Deep Black Pupil Center
        Renderer::setMaterial(0.02f, 0.02f, 0.02f, 95.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.030f);
        glScalef(0.028f, 0.028f, 0.012f);
        glutSolidSphere(1.0, 12, 12);
        glPopMatrix();

        // 5. Specular Catchlight Sparkle (Crisp white gleam on upper-right corner)
        Renderer::setMaterial(1.0f, 1.0f, 1.0f, 100.0f);
        glPushMatrix();
        glTranslatef(0.016f, 0.016f, 0.038f);
        glScalef(0.012f, 0.012f, 0.010f);
        glutSolidSphere(1.0, 8, 8);
        glPopMatrix();

        // 6. Red Light Scanner: In Red Light, a pinpoint crimson camera lens glows inside pupil
        if (isRedLight) {
            Renderer::setMaterial(1.0f, 0.10f, 0.10f, 100.0f);
            Renderer::setEmissive(1.0f, 0.05f, 0.05f, 1.0f);
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 0.032f);
            glScalef(0.014f, 0.014f, 0.010f);
            glutSolidSphere(1.0, 10, 10);
            glPopMatrix();
            Renderer::resetEmission();
        }

        glPopMatrix();
    }
}

void DollHead::drawHead(const Doll& doll) {
    bool isRed = (doll.lightState == RED_LIGHT);

    glPushMatrix();
    glRotatef(doll.headAngle, 0.0f, 1.0f, 0.0f);

    // 1. Mechanical Neck Rotating Joint Ring (Robotic doll socket)
    Renderer::setMaterial(0.18f, 0.18f, 0.20f, 60.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.42f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    Renderer::drawCylinder(0.20f, 0.20f, 0.14f, 20);
    glPopMatrix();

    // 2. Porcelain Face Cranium (Warm ivory-peach porcelain skin: Reference Image 2)
    // #F4D8CD -> R: 0.957, G: 0.847, B: 0.804
    Renderer::setMaterial(0.957f, 0.847f, 0.804f, 28.0f, 0.25f);

    // Single unified cranium ellipsoid with natural chin & jawline (NO separate double-chin ball!)
    glPushMatrix();
    glScalef(0.44f, 0.50f, 0.42f);
    glutSolidSphere(1.0, 36, 36);
    glPopMatrix();

    // 3. Delicate Korean Doll Nose (Slender, subtle bridge & tiny button tip: Reference Image 2)
    // Nose Bridge (smooth tapered sphere, not a hard box - reads as a soft subtle ridge)
    glPushMatrix();
    glTranslatef(0.0f, 0.03f, 0.408f);
    glRotatef(-18.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.026f, 0.115f, 0.034f);
    glutSolidSphere(1.0, 12, 12);
    glPopMatrix();

    // Nose Tip (Cute tiny rounded button tip)
    glPushMatrix();
    glTranslatef(0.0f, -0.04f, 0.444f);
    glScalef(0.024f, 0.022f, 0.024f);
    glutSolidSphere(1.0, 14, 14);
    glPopMatrix();

    // Subtle Nostril Wings
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(s * 0.023f, -0.046f, 0.430f);
        glScalef(0.014f, 0.012f, 0.016f);
        glutSolidSphere(1.0, 10, 10);
        glPopMatrix();
    }

    // 4. Dainty Lips: Heart-shaped Cupid's Bow with Korean Doll Orange-Red Lipstick (Image 2 close-up)
    // High-gloss doll lacquer
    Renderer::setMaterial(0.87f, 0.24f, 0.16f, 60.0f, 0.45f);

    // Upper Lip: two lobes forming a clear cupid's bow, meeting at a center dip
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(s * 0.030f, -0.152f, 0.422f);
        glScalef(0.046f, 0.018f, 0.019f);
        glutSolidSphere(1.0, 14, 14);
        glPopMatrix();
    }

    // Cupid's bow center peak (small point where the two lobes meet)
    glPushMatrix();
    glTranslatef(0.0f, -0.159f, 0.424f);
    glScalef(0.016f, 0.013f, 0.017f);
    glutSolidSphere(1.0, 10, 10);
    glPopMatrix();

    // Lower Lip (Plump gentle rounded cushion)
    glPushMatrix();
    glTranslatef(0.0f, -0.185f, 0.418f);
    glScalef(0.072f, 0.024f, 0.022f);
    glutSolidSphere(1.0, 16, 16);
    glPopMatrix();

    // 5. Soft Airbrushed Pink Blush on Cheeks (Translucent blending: NO solid lumps!)
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);

    glColor4f(0.92f, 0.45f, 0.52f, 0.32f); // 32% opacity soft circular blush
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(s * 0.20f, -0.06f, 0.395f);
        glScalef(0.075f, 0.060f, 0.015f);
        glutSolidSphere(1.0, 14, 14);
        glPopMatrix();
    }
    glPopAttrib();

    // 6. Eyes (Set in natural eye sockets)
    drawEye(-0.165f, 0.078f, 0.395f, isRed, doll.scanAlpha);
    drawEye( 0.165f, 0.078f, 0.395f, isRed, doll.scanAlpha);

    // 7. Eyebrows: Thin Curved Dark Arches (two angled segments per brow for a soft arch: Reference Image 2)
    Renderer::setMaterial(0.08f, 0.06f, 0.05f, 20.0f);
    for (int s = -1; s <= 1; s += 2) {
        // Inner segment: nearly flat, closer to nose bridge
        glPushMatrix();
        glTranslatef(s * 0.150f, 0.182f, 0.402f);
        glRotatef(s * -6.0f, 0.0f, 0.0f, 1.0f);
        Renderer::drawBox(0.062f, 0.014f, 0.014f);
        glPopMatrix();

        // Outer segment: angled upward to form the arch peak, then tapering out
        glPushMatrix();
        glTranslatef(s * 0.198f, 0.193f, 0.397f);
        glRotatef(s * -24.0f, 0.0f, 0.0f, 1.0f);
        Renderer::drawBox(0.050f, 0.012f, 0.013f);
        glPopMatrix();
    }

    // 8. HAIR SCULPTURE: Sleek Raven Black Bob with Signature Straight Bangs
    Renderer::setMaterial(0.07f, 0.06f, 0.06f, 40.0f, 0.45f);

    // Hair Skull Dome (covers top and back smoothly)
    glPushMatrix();
    glTranslatef(0.0f, 0.06f, -0.05f);
    glScalef(0.455f, 0.495f, 0.435f);
    glutSolidSphere(1.0, 32, 32);
    glPopMatrix();

    // Ears on both sides
    drawDollEar(true);  // Left ear
    drawDollEar(false); // Right ear

    // RE-APPLY RAVEN BLACK HAIR MATERIAL (drawDollEar sets skin material!)
    Renderer::setMaterial(0.07f, 0.06f, 0.06f, 40.0f, 0.45f);

    // Front Bangs Fringe (Clean straight horizontal cut across upper forehead - NO PROTRUDING BULGE!)
    glPushMatrix();
    glTranslatef(0.0f, 0.23f, 0.32f);
    glRotatef(12.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.38f, 0.065f, 0.11f);
    glutSolidCube(1.0);
    glPopMatrix();

    // 9. Purple Hair Barrette Clip (Clearly visible on right bangs, Reference Image 2, 3, 4)
    Renderer::setMaterial(0.68f, 0.22f, 0.88f, 80.0f);
    glPushMatrix();
    glTranslatef(0.18f, 0.25f, 0.36f);
    glRotatef(14.0f, 0.0f, 0.0f, 1.0f);
    Renderer::drawBox(0.10f, 0.035f, 0.016f);
    glPopMatrix();

    // 10. Low-set Pigtails with Royal Purple Beads behind Ears
    drawPigtail(true);  // Left
    drawPigtail(false); // Right

    // 11. Laser Scanning Cones during Red Light
    if (isRed && doll.headAngle > 140.0f) {
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        float pulse = 0.40f + 0.15f * sinf(doll.scanAlpha * 14.0f);

        glColor4f(1.0f, 0.08f, 0.08f, pulse);
        glPushMatrix();
        glTranslatef(-0.165f, 0.078f, 0.395f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        Renderer::drawCone(0.035f, 42.0f, 14);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.165f, 0.078f, 0.395f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        Renderer::drawCone(0.035f, 42.0f, 14);
        glPopMatrix();

        glPopAttrib();
    }

    glPopMatrix();
}
