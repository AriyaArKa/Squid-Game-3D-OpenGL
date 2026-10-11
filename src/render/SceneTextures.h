#ifndef SCENE_TEXTURES_H
#define SCENE_TEXTURES_H

#include "engine/Texture.h"

// The project's three image textures (files in assets/textures/):
//   sand     - Red Light, Green Light arena floor
//   bark     - the giant dead tree behind the doll
//   concrete - glass bridge hall walls and the two platforms
// Everything else is a plain colored material.
namespace SceneTextures {
    void loadAll();          // call once after the OpenGL context exists
    const Texture& sand();
    const Texture& bark();
    const Texture& concrete();
}

#endif // SCENE_TEXTURES_H
