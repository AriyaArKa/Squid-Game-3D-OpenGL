#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include "engine/VAO.h"
#include "engine/EBO.h"
#include "engine/Shader.h"
#include <glm/glm.hpp>
#include <vector>

// A drawable piece of geometry: vertices + indices + the VAO/VBO/EBO that describe them to
// the GPU (course tutorial 12, "Mesh Class"). Textures aren't used by this project (the doll
// and arena are built from flat-colored primitives, same as the original design), so Draw()
// takes a material color/shininess/emissive instead of a texture list.
class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    VAO vao;
    EBO ebo;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

    void Draw(Shader& shader, const glm::mat4& model,
              const glm::vec3& color, float shininess = 32.0f,
              const glm::vec3& emissive = glm::vec3(0.0f));
};

#endif // ENGINE_MESH_H
