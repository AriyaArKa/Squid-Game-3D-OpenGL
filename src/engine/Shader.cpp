#include "engine/Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

std::string Shader::readFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "SHADER: failed to open file: " << path << std::endl;
        return std::string();
    }
    std::stringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

void Shader::checkCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "SHADER COMPILE ERROR (" << type << "):\n" << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "SHADER LINK ERROR:\n" << infoLog << std::endl;
        }
    }
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::Activate() {
    glUseProgram(ID);
}

void Shader::Delete() {
    glDeleteProgram(ID);
}

void Shader::SetMat4(const char* name, const glm::mat4& value) {
    glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetVec3(const char* name, const glm::vec3& value) {
    glUniform3fv(glGetUniformLocation(ID, name), 1, glm::value_ptr(value));
}

void Shader::SetFloat(const char* name, float value) {
    glUniform1f(glGetUniformLocation(ID, name), value);
}
