#include "Doll.h"
#include "Renderer.h"
#include "Audio.h"
#include "Config.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>

namespace {
    const float PI = 3.14159265358979323846f;
    const float DEG2RAD = PI / 180.0f;

    // =========================================================================
    // 1. ORGANIC TREE RENDERING HELPERS
    // =========================================================================
    
    // Draw a perfectly connected branch segment with a joint knuckle at the tip
    void drawBranchSegment(float baseR, float tipR, float len) {
        // gluCylinder draws from Z=0 to Z=len
        Renderer::drawCylinder(baseR, tipR, len, 14);
        
        // Spherical knuckle joint at the tip to ensure 100% seamless branching without gaps
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, len);
        Renderer::drawSphere(tipR * 1.05f, 12, 12);
        glPopMatrix();
    }

    // Recursive / procedural crooked bare twig
    void drawTwig(float baseR, float len, int depth, unsigned int seed) {
        float tipR = baseR * 0.45f;
        drawBranchSegment(baseR, tipR, len);

        if (depth <= 0) return;

        glTranslatef(0.0f, 0.0f, len);

        // Fork into 2 small bare twigs with sharp angular bends (characteristic of dead hardwood)
        float angles[2][2] = {
            { 28.0f + (float)(seed % 15),   35.0f + (float)((seed * 7) % 30) },
            {-26.0f - (float)((seed * 3) % 15), -40.0f - (float)((seed * 11) % 30) }
        };

        for (int i = 0; i < 2; ++i) {
            glPushMatrix();
            glRotatef(angles[i][0], 0.0f, 1.0f, 0.0f);
            glRotatef(angles[i][1], 1.0f, 0.0f, 0.0f);
            drawTwig(tipR, len * 0.70f, depth - 1, seed * 13 + i * 7);
            glPopMatrix();
        }
    }

    // Secondary crooked bough
    void drawSubBough(float baseR, float len, float yaw, float pitch, unsigned int seed) {
        glPushMatrix();
        glRotatef(yaw, 0.0f, 0.0f, 1.0f);
        glRotatef(pitch, 1.0f, 0.0f, 0.0f);

        float midR = baseR * 0.70f;
        float seg1Len = len * 0.55f;
        drawBranchSegment(baseR, midR, seg1Len);

        // Angular bend in mid-branch
        glTranslatef(0.0f, 0.0f, seg1Len);
        glRotatef(15.0f + (float)(seed % 20), 0.0f, 1.0f, 0.0f);
        glRotatef(-10.0f - (float)((seed * 3) % 15), 1.0f, 0.0f, 0.0f);

        float tipR = midR * 0.60f;
        float seg2Len = len * 0.45f;
        drawBranchSegment(midR, tipR, seg2Len);

        // Branch into 2-3 bare twigs
        glTranslatef(0.0f, 0.0f, seg2Len);

        float twigAngles[3][2] = {
            { 32.0f,  25.0f },
            {-30.0f, -20.0f },
            {  5.0f,  38.0f }
        };

        for (int t = 0; t < 3; ++t) {
            glPushMatrix();
            glRotatef(twigAngles[t][0], 0.0f, 1.0f, 0.0f);
            glRotatef(twigAngles[t][1], 1.0f, 0.0f, 0.0f);
            drawTwig(tipR * 0.65f, len * 0.45f, 2, seed + t * 31);
            glPopMatrix();
        }

        glPopMatrix();
    }

    // Sprawling Root Buttress planted in sand
    void drawRootButtress(float angleRad, float length) {
        float rx = cosf(angleRad);
        float rz = sinf(angleRad);

        glPushMatrix();
        glTranslatef(rx * 1.3f, 0.25f, rz * 1.3f);
        glRotatef(angleRad * 180.0f / PI, 0.0f, 1.0f, 0.0f);
        glRotatef(75.0f, 1.0f, 0.0f, 0.0f); // Slanted downward into the sand

        drawBranchSegment(0.65f, 0.38f, length * 0.50f);
        glTranslatef(0.0f, 0.0f, length * 0.50f);
        glRotatef(18.0f, 1.0f, 0.0f, 0.0f); // Flatten along ground
        drawBranchSegment(0.38f, 0.12f, length * 0.50f);

        glPopMatrix();
    }

    // =========================================================================
    // 2. DOLL FACE & HEAD RENDERING HELPERS
    // =========================================================================

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

    void drawHairClip() {
        // Purple barrette clip on top-right side of bangs (Reference Image 2, 3, 4)
        Renderer::setMaterial(0.65f, 0.25f, 0.88f, 80.0f);
        glPushMatrix();
        glTranslatef(0.24f, 0.32f, 0.42f);
        glRotatef(18.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-22.0f, 0.0f, 1.0f, 0.0f);
        Renderer::drawBox(0.15f, 0.06f, 0.03f);
        glPopMatrix();
    }

    void drawEye(float x, float y, float z, bool isRedLight, float scanIntensity) {
        glPushMatrix();
        glTranslatef(x, y, z);

        // 1. Smooth White Sclera
        Renderer::setMaterial(0.97f, 0.97f, 0.97f, 30.0f);
        glPushMatrix();
        glScalef(0.078f, 0.072f, 0.030f);
        glutSolidSphere(1.0, 16, 16);
        glPopMatrix();

        // 2. Upper Eyelid & Eyelash Line (Elegantly defined curved dark lash)
        Renderer::setMaterial(0.04f, 0.04f, 0.04f, 20.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.052f, 0.022f);
        glScalef(0.082f, 0.014f, 0.016f);
        glutSolidCube(1.0);
        glPopMatrix();

        // 3. Iris (Deep glossy dark brown/black doll iris - ALWAYS dark as in reference 3D models!)
        Renderer::setMaterial(0.10f, 0.08f, 0.07f, 80.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.024f);
        glScalef(0.050f, 0.050f, 0.016f);
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

    void drawDollHead(const Doll& doll) {
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

        // 3. Delicate Korean Doll Nose (Slender bridge & tiny button tip: Reference Image 2)
        // Nose Bridge
        glPushMatrix();
        glTranslatef(0.0f, 0.03f, 0.415f);
        glRotatef(-18.0f, 1.0f, 0.0f, 0.0f);
        glScalef(0.036f, 0.13f, 0.045f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Nose Tip (Cute tiny rounded button tip)
        glPushMatrix();
        glTranslatef(0.0f, -0.04f, 0.448f);
        glScalef(0.028f, 0.026f, 0.028f);
        glutSolidSphere(1.0, 14, 14);
        glPopMatrix();

        // Subtle Nostril Wings
        for (int s = -1; s <= 1; s += 2) {
            glPushMatrix();
            glTranslatef(s * 0.026f, -0.046f, 0.432f);
            glScalef(0.016f, 0.014f, 0.018f);
            glutSolidSphere(1.0, 10, 10);
            glPopMatrix();
        }

        // 4. Dainty Lips: Elegant Cupid's Bow with Korean Doll Red Lipstick (Image 2)
        // High-gloss doll red lacquer
        Renderer::setMaterial(0.82f, 0.15f, 0.20f, 60.0f, 0.45f);

        // Upper Lip with subtle Cupid's bow dip
        glPushMatrix();
        glTranslatef(0.0f, -0.155f, 0.422f);
        glScalef(0.078f, 0.022f, 0.020f);
        glutSolidSphere(1.0, 16, 16);
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

        // 7. Eyebrows: Elegant Curved Dark Arches (Reference Image 2)
        Renderer::setMaterial(0.08f, 0.06f, 0.05f, 20.0f);
        glPushMatrix();
        glTranslatef(-0.165f, 0.185f, 0.400f);
        glRotatef(-7.0f, 0.0f, 0.0f, 1.0f);
        Renderer::drawBox(0.11f, 0.016f, 0.016f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.165f, 0.185f, 0.400f);
        glRotatef(7.0f, 0.0f, 0.0f, 1.0f);
        Renderer::drawBox(0.11f, 0.016f, 0.016f);
        glPopMatrix();

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

    void drawPleatedSkirt() {
        const int numPleats = 18;
        const float topRadius = 0.38f;
        const float botRadius = 0.65f;
        const float height = 0.58f;

        glPushMatrix();
        glTranslatef(0.0f, 1.25f, 0.0f);

        for (int i = 0; i < numPleats; ++i) {
            float a1 = i * (2.0f * PI / numPleats);
            float a2 = (i + 1) * (2.0f * PI / numPleats);
            float amid = (a1 + a2) * 0.5f;

            float rTop1 = topRadius;
            float rTop2 = topRadius;
            float rBot1 = botRadius;
            float rBot2 = botRadius;
            float rBotMid = botRadius + 0.045f;

            float shade = (i % 2 == 0) ? 0.96f : 0.85f;
            Renderer::setMaterial(0.95f * shade, 0.44f * shade, 0.08f * shade, 25.0f);

            glBegin(GL_TRIANGLES);
            glNormal3f(cosf((a1 + amid) * 0.5f), 0.3f, sinf((a1 + amid) * 0.5f));
            glVertex3f(cosf(a1) * rTop1, 0.0f, sinf(a1) * rTop1);
            glVertex3f(cosf(a1) * rBot1, -height, sinf(a1) * rBot1);
            glVertex3f(cosf(amid) * rBotMid, -height, sinf(amid) * rBotMid);

            glVertex3f(cosf(a1) * rTop1, 0.0f, sinf(a1) * rTop1);
            glVertex3f(cosf(amid) * rBotMid, -height, sinf(amid) * rBotMid);
            glVertex3f(cosf(amid) * rTop1, 0.0f, sinf(amid) * rTop1);

            glNormal3f(cosf((amid + a2) * 0.5f), 0.3f, sinf((amid + a2) * 0.5f));
            glVertex3f(cosf(amid) * rTop1, 0.0f, sinf(amid) * rTop1);
            glVertex3f(cosf(amid) * rBotMid, -height, sinf(amid) * rBotMid);
            glVertex3f(cosf(a2) * rBot2, -height, sinf(a2) * rBot2);

            glVertex3f(cosf(amid) * rTop1, 0.0f, sinf(amid) * rTop1);
            glVertex3f(cosf(a2) * rBot2, -height, sinf(a2) * rBot2);
            glVertex3f(cosf(a2) * rTop2, 0.0f, sinf(a2) * rTop2);
            glEnd();
        }

        glPopMatrix();
    }

    void drawArticulatedArm(bool isLeft) {
        float side = isLeft ? -1.0f : 1.0f;
        glPushMatrix();
        glTranslatef(side * 0.44f, 1.72f, 0.0f); // Shoulder

        // 1. Yellow Shirt Sleeve
        Renderer::setMaterial(0.98f, 0.84f, 0.12f, 25.0f);
        glPushMatrix();
        glScalef(0.14f, 0.15f, 0.14f);
        glutSolidSphere(1.0, 14, 14);
        glPopMatrix();

        // Flared Yellow Sleeve Opening Cuff Ring
        glPushMatrix();
        glTranslatef(side * 0.02f, -0.09f, 0.01f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        Renderer::drawDisk(0.08f, 0.135f, 16);
        glPopMatrix();

        // 2. Upper Arm (Angled forward toward tree (+Z) and curving around trunk)
        glRotatef(-46.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(side * 16.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(side * 6.0f, 0.0f, 0.0f, 1.0f);

        // Porcelain Upper Arm
        Renderer::setMaterial(0.957f, 0.847f, 0.804f, 25.0f);
        glPushMatrix();
        glTranslatef(0.0f, -0.15f, 0.0f);
        glScalef(0.12f, 0.30f, 0.12f);
        glutSolidCube(1.0);
        glPopMatrix();

        // 3. Ball-Joint Elbow (Mechanical doll hinge)
        glTranslatef(0.0f, -0.30f, 0.0f);
        Renderer::setMaterial(0.92f, 0.80f, 0.70f, 30.0f);
        glutSolidSphere(0.078, 14, 14);
        Renderer::setMaterial(0.35f, 0.35f, 0.35f, 50.0f);
        glPushMatrix();
        glScalef(0.082f, 0.02f, 0.082f);
        glutSolidCube(1.0);
        glPopMatrix();

        // 4. Forearm: Bends forward and wraps inward around the circular tree trunk!
        glRotatef(58.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(side * 34.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(side * -8.0f, 0.0f, 0.0f, 1.0f);

        // Porcelain Forearm
        Renderer::setMaterial(0.957f, 0.847f, 0.804f, 25.0f);
        glPushMatrix();
        glTranslatef(0.0f, -0.15f, 0.0f);
        glScalef(0.115f, 0.29f, 0.115f);
        glutSolidCube(1.0);
        glPopMatrix();

        // 5. Forearm Cuff & Mechanical Wrist Joint
        glTranslatef(0.0f, -0.29f, 0.0f);
        
        // Forearm cuff rim
        Renderer::setMaterial(0.92f, 0.80f, 0.72f, 30.0f);
        glPushMatrix();
        glScalef(0.12f, 0.035f, 0.12f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Cylindrical Wrist Pin Joint
        Renderer::setMaterial(0.88f, 0.76f, 0.68f, 30.0f);
        glPushMatrix();
        glTranslatef(0.0f, -0.025f, 0.0f);
        glutSolidSphere(0.052, 12, 12);
        glPopMatrix();

        // The iconic black wrist socket hole on outer side
        Renderer::setMaterial(0.05f, 0.05f, 0.05f, 90.0f);
        glPushMatrix();
        glTranslatef(side * 0.060f, 0.0f, 0.0f);
        glRotatef(side * -90.0f, 0.0f, 1.0f, 0.0f);
        Renderer::drawDisk(0.0f, 0.024f, 16);
        glPopMatrix();

        // 6. Porcelain Hand Gripping / Holding the Tree Trunk Bark
        Renderer::setMaterial(0.957f, 0.847f, 0.804f, 25.0f);
        glPushMatrix();
        glTranslatef(0.0f, -0.08f, 0.015f);
        // Align palm with tree trunk curvature
        glRotatef(side * -24.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(12.0f, 1.0f, 0.0f, 0.0f);

        // Palm block
        glPushMatrix();
        glScalef(0.11f, 0.085f, 0.045f);
        glutSolidCube(1.0);
        glPopMatrix();

        // 4 Distinct Fingers gripping the tree bark
        for (int f = 0; f < 4; ++f) {
            float fx = -0.038f + f * 0.025f;
            glPushMatrix();
            glTranslatef(fx, -0.070f, 0.008f);
            glRotatef(side * (-10.0f - f * 3.0f), 0.0f, 1.0f, 0.0f);
            glScalef(0.020f, 0.065f, 0.025f);
            glutSolidCube(1.0);
            glPopMatrix();
        }

        // Thumb resting on upper bark
        glPushMatrix();
        glTranslatef(side * -0.045f, -0.020f, 0.012f);
        glRotatef(side * -28.0f, 0.0f, 0.0f, 1.0f);
        glScalef(0.022f, 0.045f, 0.025f);
        glutSolidCube(1.0);
        glPopMatrix();

        glPopMatrix(); // End Hand
        glPopMatrix(); // End Arm
    }

    void drawDollBody() {
        // 1. Neck Post
        Renderer::setMaterial(0.957f, 0.847f, 0.804f, 25.0f);
        glPushMatrix();
        glTranslatef(0.0f, 2.05f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        Renderer::drawCylinder(0.18f, 0.18f, 0.28f, 16);
        glPopMatrix();

        // 2. White Peter Pan Collar
        Renderer::setMaterial(0.98f, 0.98f, 0.98f, 35.0f);
        glPushMatrix();
        glTranslatef(0.0f, 1.96f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        Renderer::drawDisk(0.18f, 0.38f, 24);
        glPopMatrix();

        // 3. Yellow Shirt Bodice
        Renderer::setMaterial(0.98f, 0.84f, 0.12f, 25.0f);
        glPushMatrix();
        glTranslatef(0.0f, 1.70f, 0.0f);
        glScalef(0.64f, 0.50f, 0.44f);
        glutSolidCube(1.0);
        glPopMatrix();

        // 4. Orange Pinafore Overall Dress
        Renderer::setMaterial(0.95f, 0.44f, 0.08f, 30.0f);

        // Dress Bodice
        glPushMatrix();
        glTranslatef(0.0f, 1.55f, 0.01f);
        glScalef(0.60f, 0.45f, 0.45f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Shoulder Straps
        for (int s = -1; s <= 1; s += 2) {
            glPushMatrix();
            glTranslatef(s * 0.19f, 1.78f, 0.0f);
            glScalef(0.11f, 0.32f, 0.46f);
            glutSolidCube(1.0);
            glPopMatrix();
        }

        // 5. Pleated Flared Skirt
        drawPleatedSkirt();

        // 6. Articulated Arms
        drawArticulatedArm(true);
        drawArticulatedArm(false);

        // 7. Legs: Thighs + White Knee Socks + Mary Jane Shoes
        for (int side = -1; side <= 1; side += 2) {
            float lx = side * 0.19f;

            // Porcelain upper thigh
            Renderer::setMaterial(0.957f, 0.847f, 0.804f, 25.0f);
            glPushMatrix();
            glTranslatef(lx, 0.85f, 0.0f);
            glScalef(0.18f, 0.32f, 0.18f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Ball-Joint Knee Crease
            Renderer::setMaterial(0.92f, 0.80f, 0.70f, 30.0f);
            glPushMatrix();
            glTranslatef(lx, 0.68f, 0.0f);
            glutSolidSphere(0.09, 12, 12);
            glPopMatrix();

            // White Knee-High Sock
            Renderer::setMaterial(0.98f, 0.98f, 0.98f, 25.0f);
            glPushMatrix();
            glTranslatef(lx, 0.64f, 0.0f);
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            Renderer::drawCylinder(0.11f, 0.10f, 0.05f, 16);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(lx, 0.36f, 0.0f);
            glScalef(0.17f, 0.52f, 0.17f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Black Mary Jane Shoes with Buckle (Reference Image 2, 3)
            Renderer::setMaterial(0.10f, 0.10f, 0.10f, 50.0f);
            glPushMatrix();
            glTranslatef(lx, 0.09f, 0.04f);
            glScalef(0.20f, 0.16f, 0.30f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Shoe sole
            Renderer::setMaterial(0.05f, 0.05f, 0.05f, 30.0f);
            glPushMatrix();
            glTranslatef(lx, 0.02f, 0.04f);
            glScalef(0.21f, 0.04f, 0.31f);
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }
}

void DollManager::init(Doll& doll) {
    doll.x = DOLL_X;
    doll.y = DOLL_Y;
    doll.z = DOLL_Z;
    doll.headAngle = 0.0f; // 0 = facing tree/green light, 180 = facing players/red light
    doll.targetAngle = 0.0f;
    doll.lightState = GREEN_LIGHT;
    doll.chantDuration = 5.0f;
    doll.stateTimer = 5.0f;
    doll.chantProgress = 0.0f;
    doll.isTurning = false;
    doll.scanAlpha = 0.0f;
}

void DollManager::update(Doll& doll, float dt) {
    doll.stateTimer -= dt;
    doll.scanAlpha += dt;

    if (doll.lightState == GREEN_LIGHT) {
        doll.targetAngle = 0.0f;
        doll.chantProgress += dt / doll.chantDuration;

        if (doll.stateTimer <= 0.0f) {
            doll.lightState = RED_LIGHT;
            doll.targetAngle = 180.0f;
            doll.isTurning = true;
            doll.stateTimer = 3.2f + (rand() % 2000) / 1000.0f;
            AudioManager::pauseSong();
            AudioManager::playHeadTurnClick();
        }
    } else {
        doll.targetAngle = 180.0f;

        if (doll.stateTimer <= 0.0f) {
            doll.lightState = GREEN_LIGHT;
            doll.targetAngle = 0.0f;
            doll.isTurning = true;
            doll.chantDuration = 4.8f + (rand() % 1400) / 1000.0f;
            doll.stateTimer = doll.chantDuration;
            doll.chantProgress = 0.0f;

            AudioManager::restartSong();
            AudioManager::playHeadTurnClick();
        }
    }

    float diff = doll.targetAngle - doll.headAngle;
    if (fabs(diff) > 0.4f) {
        doll.headAngle += diff * 15.0f * dt;
    } else {
        doll.headAngle = doll.targetAngle;
        doll.isTurning = false;
    }
}

void DollManager::draw(const Doll& doll) {
    // 1. Draw the Colossal Young-hee Doll standing on the arena sand
    glPushMatrix();
    glTranslatef(doll.x, doll.y, doll.z);
    glScalef(DOLL_SCALE, DOLL_SCALE, DOLL_SCALE);

    // Ground shadow beneath the doll
    Renderer::drawGroundShadow(0.0f, 0.0f, 0.95f, 0.85f, 0.45f);

    // Body
    drawDollBody();

    // Head (turns with headAngle)
    glPushMatrix();
    glTranslatef(0.0f, 2.50f, 0.0f);
    drawDollHead(doll);
    glPopMatrix();

    glPopMatrix();

    // 2. Draw the Real, Seamless Colossal Dead Tree directly in front of the doll
    // Positioned at (doll.x, doll.y, doll.z + 1.85f) so the doll hugs the trunk with both hands.
    // When Red Light comes, ONLY her face/head rotates 180° to look back at the players!
    drawDeadTree(doll.x, doll.y, doll.z + 1.85f);
}

void DollManager::drawDeadTree(float x, float y, float z) {
    // =========================================================================
    // REAL CONTINUOUS 3D GNARLED DEAD TREE (Zero Gaps, 360° Canopy Branching)
    // =========================================================================
    glPushMatrix();
    glTranslatef(x, y, z);

    // Deep weathered hardwood bark color (rich fibrous organic tones)
    const float barkR = 0.24f, barkG = 0.20f, barkB = 0.16f;
    Renderer::setMaterial(barkR, barkG, barkB, 15.0f);

    // 1. Sprawling Root Buttresses radiating into the sand in 360 degrees
    for (int i = 0; i < 8; ++i) {
        float ang = i * (2.0f * PI / 8.0f);
        drawRootButtress(ang, 3.2f);
    }

    // 2. Continuous Lower Trunk (glRotatef -90 on X so Z points straight UP!)
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // Trunk Segment 1: Base to 2.2m
    drawBranchSegment(1.45f, 1.25f, 2.2f);

    // Trunk Segment 2: 2.2m to 4.2m (curving slightly)
    glTranslatef(0.0f, 0.0f, 2.2f);
    glRotatef(3.5f, 1.0f, 0.0f, 0.0f);
    glRotatef(-4.0f, 0.0f, 1.0f, 0.0f);
    drawBranchSegment(1.25f, 1.05f, 2.0f);

    // Trunk Segment 3: 4.2m to 6.0m (tapering to the Grand Fork)
    glTranslatef(0.0f, 0.0f, 2.0f);
    glRotatef(-4.5f, 1.0f, 0.0f, 0.0f);
    glRotatef(5.0f, 0.0f, 1.0f, 0.0f);
    drawBranchSegment(1.05f, 0.88f, 1.8f);

    // AT HEIGHT 6.0m: THE GRAND FORK
    // Huge continuous wooden knuckle sphere
    glTranslatef(0.0f, 0.0f, 1.8f);
    Renderer::drawSphere(0.92f, 16, 16);

    // =========================================================================
    // 3. FIVE MAJOR GNARLED BOUGHS SPREADING ACROSS ALL 360 DEGREES!
    // =========================================================================

    // BOUGH 1: Major Left Bough (Spreading Left and Forward)
    drawSubBough(0.56f, 3.6f, -48.0f, 38.0f, 101);

    // BOUGH 2: Major Right Bough (Spreading Right and Forward)
    drawSubBough(0.56f, 3.6f,  48.0f, 38.0f, 202);

    // BOUGH 3: Major Rear-Left Bough (Spreading Left and Back toward Wall)
    drawSubBough(0.52f, 3.4f, -135.0f, 32.0f, 303);

    // BOUGH 4: Major Rear-Right Bough (Spreading Right and Back toward Wall)
    drawSubBough(0.52f, 3.4f,  135.0f, 32.0f, 404);

    // BOUGH 5: Forward-Upper Canopy Bough (Spreading directly over Doll's head)
    drawSubBough(0.50f, 3.2f, 180.0f, 28.0f, 505);

    // BOUGH 6: Center-Top Towering Spire (Reaches 14-15 meters tall!)
    glPushMatrix();
    glRotatef(8.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(12.0f, 0.0f, 1.0f, 0.0f);
    drawBranchSegment(0.65f, 0.42f, 3.5f);

    glTranslatef(0.0f, 0.0f, 3.5f);
    // Spire crown forks into 3 high twigs
    float spireAngles[3][2] = {
        { 18.0f,  22.0f },
        {-20.0f, -18.0f },
        { 32.0f, -24.0f }
    };
    for (int s = 0; s < 3; ++s) {
        glPushMatrix();
        glRotatef(spireAngles[s][0], 0.0f, 1.0f, 0.0f);
        glRotatef(spireAngles[s][1], 1.0f, 0.0f, 0.0f);
        drawTwig(0.24f, 2.8f, 2, 606 + s * 17);
        glPopMatrix();
    }
    glPopMatrix();

    glPopMatrix(); // End Trunk

    glPopMatrix(); // End Tree
}
