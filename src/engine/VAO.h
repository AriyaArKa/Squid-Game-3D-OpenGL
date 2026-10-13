#ifndef ENGINE_VAO_H
#define ENGINE_VAO_H

#include "engine/VBO.h"
#include <glad/gl.h>

// Wraps a Vertex Array Object and links VBO attributes to shader locations
// (course tutorial 4, "Organizing").
class VAO {
public:
    GLuint ID;

    VAO();

    void LinkAttrib(VBO& vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset);
    void Bind();
    void Unbind();
    void Delete();
};

#endif // ENGINE_VAO_H
