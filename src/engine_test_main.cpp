// Milestone 2 checkpoint: a minimal GLFW + GLAD + GLSL-shader pipeline rendering one
// Blinn-Phong-lit rotating cube, built from the new engine/{Shader,VAO,VBO,EBO,Camera,Mesh}
// classes. This is NOT part of the game - it exists only to prove the modern-OpenGL
// foundation works before the game content gets ported onto it in later milestones.
// Build separately (see the milestone 2 build command); not wired into tasks.json yet.

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "engine/Shader.h"
#include "engine/Mesh.h"
#include "engine/Camera.h"

namespace {
    void framebufferSizeCallback(GLFWwindow*, int w, int h) {
        glViewport(0, 0, w, h);
    }

    Mesh buildUnitCube() {
        // 24 vertices (4 per face) so each face gets its own flat normal.
        std::vector<Vertex> v = {
            // Front (+Z)
            {{-0.5f,-0.5f, 0.5f}, {0,0,1}, {1,1,1}}, {{0.5f,-0.5f, 0.5f}, {0,0,1}, {1,1,1}},
            {{ 0.5f, 0.5f, 0.5f}, {0,0,1}, {1,1,1}}, {{-0.5f, 0.5f, 0.5f}, {0,0,1}, {1,1,1}},
            // Back (-Z)
            {{ 0.5f,-0.5f,-0.5f}, {0,0,-1}, {1,1,1}}, {{-0.5f,-0.5f,-0.5f}, {0,0,-1}, {1,1,1}},
            {{-0.5f, 0.5f,-0.5f}, {0,0,-1}, {1,1,1}}, {{ 0.5f, 0.5f,-0.5f}, {0,0,-1}, {1,1,1}},
            // Left (-X)
            {{-0.5f,-0.5f,-0.5f}, {-1,0,0}, {1,1,1}}, {{-0.5f,-0.5f, 0.5f}, {-1,0,0}, {1,1,1}},
            {{-0.5f, 0.5f, 0.5f}, {-1,0,0}, {1,1,1}}, {{-0.5f, 0.5f,-0.5f}, {-1,0,0}, {1,1,1}},
            // Right (+X)
            {{0.5f,-0.5f, 0.5f}, {1,0,0}, {1,1,1}}, {{0.5f,-0.5f,-0.5f}, {1,0,0}, {1,1,1}},
            {{0.5f, 0.5f,-0.5f}, {1,0,0}, {1,1,1}}, {{0.5f, 0.5f, 0.5f}, {1,0,0}, {1,1,1}},
            // Top (+Y)
            {{-0.5f, 0.5f, 0.5f}, {0,1,0}, {1,1,1}}, {{0.5f, 0.5f, 0.5f}, {0,1,0}, {1,1,1}},
            {{0.5f, 0.5f,-0.5f}, {0,1,0}, {1,1,1}}, {{-0.5f, 0.5f,-0.5f}, {0,1,0}, {1,1,1}},
            // Bottom (-Y)
            {{-0.5f,-0.5f,-0.5f}, {0,-1,0}, {1,1,1}}, {{0.5f,-0.5f,-0.5f}, {0,-1,0}, {1,1,1}},
            {{0.5f,-0.5f, 0.5f}, {0,-1,0}, {1,1,1}}, {{-0.5f,-0.5f, 0.5f}, {0,-1,0}, {1,1,1}},
        };
        std::vector<GLuint> idx;
        for (GLuint f = 0; f < 6; ++f) {
            GLuint base = f * 4;
            idx.push_back(base + 0); idx.push_back(base + 1); idx.push_back(base + 2);
            idx.push_back(base + 0); idx.push_back(base + 2); idx.push_back(base + 3);
        }
        return Mesh(v, idx);
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(900, 650, "Engine Test - Milestone 2 Checkpoint", nullptr, nullptr);
    if (!window) {
        std::cerr << "GLFW window creation failed\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "GLAD load failed\n";
        return -1;
    }

    glViewport(0, 0, 900, 650);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.10f, 0.14f, 1.0f);

    Shader shader("src/shaders/default.vert", "src/shaders/default.frag");
    Mesh cube = buildUnitCube();
    Camera camera(900, 650, glm::vec3(0.0f, 1.2f, 3.5f));

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.Inputs(window);
        camera.UpdateMatrix(55.0f, 0.1f, 100.0f);
        camera.Apply(shader);

        shader.Activate();
        shader.SetVec3("lightDir", glm::vec3(-0.4f, -1.0f, -0.3f));
        shader.SetVec3("lightColor", glm::vec3(1.0f, 0.98f, 0.92f));

        float t = (float)glfwGetTime();
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), t * 0.8f, glm::vec3(0.3f, 1.0f, 0.0f));
        cube.Draw(shader, model, glm::vec3(0.94f, 0.16f, 0.48f), 48.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
