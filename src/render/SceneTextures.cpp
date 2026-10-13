#include "render/SceneTextures.h"

namespace {
    Texture sandTexture;
    Texture barkTexture;
    Texture concreteTexture;
}

void SceneTextures::loadAll() {
    sandTexture.Load("assets/textures/sand.png");
    barkTexture.Load("assets/textures/bark.png");
    concreteTexture.Load("assets/textures/concrete.png");
}

const Texture& SceneTextures::sand()     { return sandTexture; }
const Texture& SceneTextures::bark()     { return barkTexture; }
const Texture& SceneTextures::concrete() { return concreteTexture; }
