#ifndef TOOLS_SHOWCASE_H
#define TOOLS_SHOWCASE_H

#include "engine/Shader.h"
#include "engine/Camera.h"
#include <string>
#include <vector>

// Renders ONE model by itself on a plain background, automatically framed by the camera.
// Used to generate the report's figures ("full object" next to "object split into its
// basic shapes") straight from the real drawing code, so the pictures always match it.
namespace Showcase {
    // Names of every item that can be rendered (e.g. "player", "guard", "prim_box", ...).
    std::vector<std::string> itemNames();

    // Draws `item` into the viewport (x, y, w, h) of the current framebuffer.
    // parts = false: normal look.  parts = true: every part painted by its shape kind and
    // pushed slightly apart (exploded view). Returns a short text with the number of parts
    // of each shape kind, e.g. "Box (cube)=24\nSphere=6\n".
    std::string render(const std::string& item, bool parts, Shader& shader, Camera& camera,
                       int x, int y, int w, int h);
}

// Writes the pixels of a framebuffer region to a 24-bit .bmp file.
namespace Capture {
    bool saveBMP(const std::string& path, int x, int y, int w, int h);
}

#endif // TOOLS_SHOWCASE_H
