#ifndef ENGINE_TEXTURE_H
#define ENGINE_TEXTURE_H

#include <glad/gl.h>
#include <string>

// A 2D image texture loaded from a PNG/JPG file (course tutorial 6, "Textures").
// The image is uploaded once with mipmaps and GL_REPEAT wrapping, so a surface can tile
// it many times by scaling its texture coordinates (see GLCompat::setTexture).
class Texture {
public:
    GLuint ID = 0;
    int width = 0;
    int height = 0;

    // Returns false (and leaves ID = 0) if the file cannot be read.
    bool Load(const std::string& path);

    void Bind(GLuint unit = 0) const;
    void Delete();
    bool IsLoaded() const { return ID != 0; }
};

#endif // ENGINE_TEXTURE_H
