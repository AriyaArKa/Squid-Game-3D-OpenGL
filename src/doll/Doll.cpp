#include "doll/Doll.h"
#include "doll/DollHead.h"
#include "doll/DollBody.h"
#include "doll/DollTree.h"
#include "render/Renderer.h"
#include "audio/Audio.h"
#include "core/Config.h"
#include <cmath>
#include <cstdlib>

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
            // Hard stop (not pause): MCI's "pause" on a compressed MP3 lets already-decoded
            // audio drain out over the next moment, so the song lingers past the red-light cue.
            // "stop" cuts immediately, and the song always restarts from 0 on the next green light anyway.
            AudioManager::stopSong();
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
    drawDollOnly(doll);

    // 2. Draw the Real, Seamless Colossal Dead Tree directly in front of the doll
    // Positioned at (doll.x, doll.y, doll.z + 1.85f) so the doll hugs the trunk with both hands.
    // When Red Light comes, ONLY her face/head rotates 180° to look back at the players!
    DollTree::drawDeadTree(doll.x, doll.y, doll.z + 1.85f);
}

void DollManager::drawDollOnly(const Doll& doll) {
    glPushMatrix();
    glTranslatef(doll.x, doll.y, doll.z);
    glScalef(DOLL_SCALE, DOLL_SCALE, DOLL_SCALE);

    // Ground shadow beneath the doll
    Renderer::drawGroundShadow(0.0f, 0.0f, 0.95f, 0.85f, 0.45f);

    // Body
    DollBody::drawBody();

    // Head (turns with headAngle)
    glPushMatrix();
    glTranslatef(0.0f, 2.50f, 0.0f);
    DollHead::drawHead(doll);
    glPopMatrix();

    glPopMatrix();
}
