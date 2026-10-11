#ifndef ENGINE_VBO_H
#define ENGINE_VBO_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

// One vertex: position + normal (for lighting) + color (so flat-shaded primitives and
// vertex-colored meshes like the sky walls both work with a single shader/pipeline)
// + texCoord (u,v) for the few texture-mapped surfaces (sand, bark, concrete).
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoord;
};

// Wraps a Vertex Buffer Object (course tutorial 3/4 - "Indices"/"Organizing").
class VBO {
public:
    GLuint ID;

    VBO(const std::vector<Vertex>& vertices);

    void Bind();
    void Unbind();
    void Delete();
};

#endif // ENGINE_VBO_H
