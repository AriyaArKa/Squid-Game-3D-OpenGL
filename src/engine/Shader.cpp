#include "engine/Shader.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

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

    // Pre-cache high frequency uniform locations to avoid string allocations in draw loop
    locModel = glGetUniformLocation(ID, "model");
    locMaterialColor = glGetUniformLocation(ID, "materialColor");
    locShininess = glGetUniformLocation(ID, "shininess");
    locMaterialSpecularStrength = glGetUniformLocation(ID, "materialSpecularStrength");
    locEmissive = glGetUniformLocation(ID, "emissive");
    locAlphaValue = glGetUniformLocation(ID, "alphaValue");
    locUnlit = glGetUniformLocation(ID, "unlit");
    locUseTexture = glGetUniformLocation(ID, "useTexture");
    locDiffuseTexture = glGetUniformLocation(ID, "diffuseTexture");
    locTexRepeat = glGetUniformLocation(ID, "texRepeat");
}

void Shader::Activate() {
    glUseProgram(ID);
}

void Shader::Delete() {
    glDeleteProgram(ID);
}

GLint Shader::loc(const char* name) {
    auto it = uniformCache.find(name);
    if (it != uniformCache.end()) return it->second;
    GLint location = glGetUniformLocation(ID, name);
    uniformCache.emplace(name, location);
    return location;
}

void Shader::SetModel(const glm::mat4& model) {
    if (locModel >= 0) {
        glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
    }
}

void Shader::SetMat4(const char* name, const glm::mat4& value) {
    if (strcmp(name, "model") == 0) {
        SetModel(value);
        return;
    }
    glUniformMatrix4fv(loc(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat3(const char* name, const glm::mat3& value) {
    glUniformMatrix3fv(loc(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetVec3(const char* name, const glm::vec3& value) {
    if (strcmp(name, "materialColor") == 0 && locMaterialColor >= 0) {
        glUniform3fv(locMaterialColor, 1, glm::value_ptr(value));
        return;
    }
    if (strcmp(name, "emissive") == 0 && locEmissive >= 0) {
        glUniform3fv(locEmissive, 1, glm::value_ptr(value));
        return;
    }
    glUniform3fv(loc(name), 1, glm::value_ptr(value));
}

void Shader::SetVec2(const char* name, const glm::vec2& value) {
    if (strcmp(name, "texRepeat") == 0 && locTexRepeat >= 0) {
        glUniform2fv(locTexRepeat, 1, glm::value_ptr(value));
        return;
    }
    glUniform2fv(loc(name), 1, glm::value_ptr(value));
}

void Shader::SetFloat(const char* name, float value) {
    if (strcmp(name, "shininess") == 0 && locShininess >= 0) {
        glUniform1f(locShininess, value);
        return;
    }
    if (strcmp(name, "materialSpecularStrength") == 0 && locMaterialSpecularStrength >= 0) {
        glUniform1f(locMaterialSpecularStrength, value);
        return;
    }
    if (strcmp(name, "alphaValue") == 0 && locAlphaValue >= 0) {
        glUniform1f(locAlphaValue, value);
        return;
    }
    glUniform1f(loc(name), value);
}

void Shader::SetBool(const char* name, bool value) {
    if (strcmp(name, "unlit") == 0 && locUnlit >= 0) {
        glUniform1i(locUnlit, value ? 1 : 0);
        return;
    }
    if (strcmp(name, "useTexture") == 0 && locUseTexture >= 0) {
        glUniform1i(locUseTexture, value ? 1 : 0);
        return;
    }
    glUniform1i(loc(name), value ? 1 : 0);
}

void Shader::SetInt(const char* name, int value) {
    if (strcmp(name, "diffuseTexture") == 0 && locDiffuseTexture >= 0) {
        glUniform1i(locDiffuseTexture, value);
        return;
    }
    glUniform1i(loc(name), value);
}
