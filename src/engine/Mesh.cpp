#include "engine/Mesh.h"
#include <cstddef>

Mesh::Mesh(const std::vector<Vertex>& vertices_, const std::vector<GLuint>& indices_, GLenum drawMode_)
    : vertices(vertices_), indices(indices_), ebo(indices_), drawMode(drawMode_) {
    vao.Bind();

    VBO vbo(vertices);
    vboID = vbo.ID;
    ebo.Bind(); // re-associate with the now-bound VAO (must happen after vao.Bind())

    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    vao.LinkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));
    vao.LinkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // Unbind the VAO first - unbinding the EBO while a VAO is still bound would clear
    // that VAO's element-buffer association.
    vao.Unbind();
    vbo.Unbind();
}

void Mesh::Draw(Shader& shader, const glm::mat4& model,
                const glm::vec3& color, float shininess, const glm::vec3& emissive,
                float alpha, bool unlit, float specularStrength) {
    shader.Activate();
    shader.SetMat4("model", model);
    shader.SetVec3("materialColor", color);
    shader.SetFloat("shininess", shininess);
    shader.SetVec3("emissive", emissive);
    shader.SetFloat("alphaValue", alpha);
    shader.SetBool("unlit", unlit);
    shader.SetFloat("materialSpecularStrength", specularStrength);
    shader.SetBool("useTexture", false);

    vao.Bind();
    glDrawElements(drawMode, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    vao.Unbind();
}

void Mesh::DrawGeometry(Shader& shader, const glm::mat4& model) {
    shader.SetMat4("model", model);
    vao.Bind();
    glDrawElements(drawMode, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
    vao.Unbind();
}

void Mesh::Delete() {
    vao.Delete();
    ebo.Delete();
    if (vboID != 0) {
        glDeleteBuffers(1, &vboID);
        vboID = 0;
    }
}
