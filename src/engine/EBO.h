#ifndef ENGINE_EBO_H
#define ENGINE_EBO_H

#include <glad/gl.h>
#include <vector>

// Wraps an Element Buffer Object (indices) - course tutorial 3, "Indices".
class EBO {
public:
    GLuint ID;

    EBO(const std::vector<GLuint>& indices);

    void Bind();
    void Unbind();
    void Delete();
};

#endif // ENGINE_EBO_H
