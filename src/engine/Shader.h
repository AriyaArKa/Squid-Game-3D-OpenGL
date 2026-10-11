#ifndef ENGINE_SHADER_H
#define ENGINE_SHADER_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

// Compiles a GLSL vertex+fragment shader pair into a linked program (course tutorial 5 -
// "Shaders"). Kept intentionally small: read source, compile, link, check errors, expose ID.
class Shader {
public:
    GLuint ID;

    Shader(const char* vertexPath, const char* fragmentPath);

    void Activate();
    void Delete();

    void SetMat4(const char* name, const glm::mat4& value);
    void SetVec3(const char* name, const glm::vec3& value);
    void SetFloat(const char* name, float value);
    void SetBool(const char* name, bool value);
    void SetInt(const char* name, int value);
    void SetVec2(const char* name, const glm::vec2& value);

private:
    // glGetUniformLocation is a string lookup inside the driver; the scene issues thousands
    // of draws per frame, so each location is looked up once and remembered.
    std::unordered_map<std::string, GLint> uniformCache;
    GLint loc(const char* name);

    static std::string readFile(const char* path);
    static void checkCompileErrors(GLuint shader, const std::string& type);
};

#endif // ENGINE_SHADER_H
