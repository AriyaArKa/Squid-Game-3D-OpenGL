#include "doll/DollBody.h"
#include "render/Renderer.h"
#include <GL/glut.h>
#include <cmath>

namespace {
    const float PI = 3.14159265358979323846f;

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

        // 6. Porcelain Ball-Jointed Hand (BJD style, matching shoulder/elbow/knee joints) Gripping the Tree Trunk Bark
        Renderer::setMaterial(0.957f, 0.847f, 0.804f, 25.0f);
        glPushMatrix();
        glTranslatef(0.0f, -0.08f, 0.015f);
        // Align palm with tree trunk curvature
        glRotatef(side * -24.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(12.0f, 1.0f, 0.0f, 0.0f);

        // Rounded porcelain palm (softened, not a hard block)
        glPushMatrix();
        glScalef(0.11f, 0.085f, 0.045f);
        glutSolidSphere(1.0, 12, 10);
        glPopMatrix();

        // 4 Ball-jointed fingers gripping the tree bark: knuckle joint + 2 phalanges + mid joint each
        for (int f = 0; f < 4; ++f) {
            float fx = -0.038f + f * 0.025f;
            glPushMatrix();
            glTranslatef(fx, -0.070f, 0.008f);
            glRotatef(side * (-10.0f - f * 3.0f), 0.0f, 1.0f, 0.0f);

            // Knuckle ball joint at the base of the finger
            glutSolidSphere(0.013, 8, 8);

            // Proximal phalange
            glPushMatrix();
            glTranslatef(0.0f, -0.022f, 0.0f);
            glScalef(0.017f, 0.030f, 0.021f);
            glutSolidCube(1.0);
            glPopMatrix();

            // Mid-finger ball joint, curling the distal phalange around the bark
            glPushMatrix();
            glTranslatef(0.0f, -0.040f, 0.0f);
            glRotatef(16.0f, 1.0f, 0.0f, 0.0f);
            glutSolidSphere(0.011, 8, 8);

            glPushMatrix();
            glTranslatef(0.0f, -0.020f, 0.004f);
            glScalef(0.014f, 0.026f, 0.018f);
            glutSolidCube(1.0);
            glPopMatrix();

            glPopMatrix(); // mid joint
            glPopMatrix(); // finger
        }

        // Thumb: knuckle + mid ball joints, resting on upper bark
        glPushMatrix();
        glTranslatef(side * -0.045f, -0.020f, 0.012f);
        glRotatef(side * -28.0f, 0.0f, 0.0f, 1.0f);

        glutSolidSphere(0.014, 8, 8);

        glPushMatrix();
        glTranslatef(0.0f, -0.024f, 0.0f);
        glScalef(0.020f, 0.032f, 0.022f);
        glutSolidCube(1.0);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, -0.042f, 0.003f);
        glRotatef(-10.0f, 1.0f, 0.0f, 0.0f);
        glutSolidSphere(0.010, 8, 8);

        glPushMatrix();
        glTranslatef(0.0f, -0.018f, 0.0f);
        glScalef(0.016f, 0.024f, 0.018f);
        glutSolidCube(1.0);
        glPopMatrix();

        glPopMatrix(); // mid joint

        glPopMatrix(); // thumb

        glPopMatrix(); // End Hand
        glPopMatrix(); // End Arm
    }
}

void DollBody::drawBody() {
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
