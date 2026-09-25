#include "engine/Mesh.h"
#include <cstddef>

Mesh::Mesh(const std::vector<Vertex>& vertices_, const std::vector<GLuint>& indices_)
    : vertices(vertices_), indices(indices_), ebo(indices_) {
    vao.Bind();

    VBO vbo(vertices);
    ebo.Bind(); // re-associate with the now-bound VAO (must happen after vao.Bind())

    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    vao.LinkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));

    // Unbind the VAO first - unbinding the EBO while a VAO is still bound would clear
    // that VAO's element-buffer association.
    vao.Unbind();
    vbo.Unbind();
}

void Mesh::Draw(Shader& shader, const glm::mat4& model,
                const glm::vec3& color, float shininess, const glm::vec3& emissive) {
    shader.Activate();
    shader.SetMat4("model", model);
    shader.SetVec3("materialColor", color);
    shader.SetFloat("shininess", shininess);
    shader.SetVec3("emissive", emissive);

    vao.Bind();
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    vao.Unbind();
}
