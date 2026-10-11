#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include "engine/VAO.h"
#include "engine/EBO.h"
#include "engine/Shader.h"
#include <glm/glm.hpp>
#include <vector>

// A drawable piece of geometry: vertices + indices + the VAO/VBO/EBO that describe them to
// the GPU (course tutorial 12, "Mesh Class"). Most surfaces are flat-colored materials, so
// Draw() takes a material color/shininess/emissive; the few textured surfaces bind their
// texture through GLCompat::setTexture() and then call DrawGeometry().
class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    VAO vao;
    EBO ebo;
    GLuint vboID = 0;
    GLenum drawMode;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, GLenum drawMode = GL_TRIANGLES);

    void Draw(Shader& shader, const glm::mat4& model,
              const glm::vec3& color, float shininess = 32.0f,
              const glm::vec3& emissive = glm::vec3(0.0f),
              float alpha = 1.0f, bool unlit = false,
              float specularStrength = 0.35f);

    // Uploads only the model matrix and issues the draw call. Used by GLCompat, which has
    // already uploaded the full material (color, texture, emission...) for this draw.
    void DrawGeometry(Shader& shader, const glm::mat4& model);

    // Frees the VAO/EBO (and the VBO buffer created alongside them). Only call this for
    // one-off meshes (e.g. GLCompat's per-call immediate-mode geometry) - the shared/cached
    // meshes in Primitives.h live for the whole program and are never deleted.
    void Delete();
};

#endif // ENGINE_MESH_H
