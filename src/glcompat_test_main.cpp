// Milestone 3 checkpoint: port EnvironmentManager::drawBoundaryLines (the legacy
// environment/Environment.cpp) onto the new GLCompat shim as a smoke test, almost
// character-for-character - proving the pushMatrix/translatef/scalef/drawBox +
// setMaterial/setEmissive call shape genuinely carries over with minimal changes.
// Temporary/throwaway, like engine_test_main.cpp; not wired into tasks.json.

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include "engine/Shader.h"
#include "engine/Camera.h"
#include "engine/GLCompat.h"

namespace {
    const float ARENA_HALF_WIDTH = 24.0f;
    const float START_Z = -52.0f;
    const float FINISH_Z = 45.0f;

    void framebufferSizeCallback(GLFWwindow*, int w, int h) {
        glViewport(0, 0, w, h);
    }

    // Ported 1:1 from EnvironmentManager::drawBoundaryLines (src/environment/Environment.cpp),
    // just with GLCompat:: in front of every call instead of raw gl*/Renderer:: calls.
    void drawBoundaryLinesCompat() {
        // Starting Red Line (behind contestants at START_Z)
        GLCompat::setMaterial(0.95f, 0.15f, 0.15f, 40.0f);
        GLCompat::pushMatrix();
        GLCompat::translatef(0.0f, 0.01f, START_Z);
        GLCompat::scalef(ARENA_HALF_WIDTH * 2.0f, 0.01f, 0.35f);
        GLCompat::drawBox(1.0f, 1.0f, 1.0f);
        GLCompat::popMatrix();

        // Finish Hot Pink Line (in front of doll at FINISH_Z), glowing via setEmissive
        GLCompat::setMaterial(0.96f, 0.18f, 0.52f, 50.0f);
        GLCompat::setEmissive(0.95f, 0.12f, 0.45f, 0.5f);
        GLCompat::pushMatrix();
        GLCompat::translatef(0.0f, 0.01f, FINISH_Z);
        GLCompat::scalef(ARENA_HALF_WIDTH * 2.0f, 0.01f, 0.45f);
        GLCompat::drawBox(1.0f, 1.0f, 1.0f);
        GLCompat::popMatrix();
        GLCompat::resetEmission();
    }
}

int main() {
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(900, 650, "GLCompat Test - Milestone 3 Checkpoint", nullptr, nullptr);
    if (!window) { std::cerr << "window creation failed\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) { std::cerr << "GLAD load failed\n"; return -1; }

    glViewport(0, 0, 900, 650);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.55f, 0.78f, 0.95f, 1.0f);

    Shader shader("src/shaders/default.vert", "src/shaders/default.frag");
    // Framed on the glowing pink finish line, close enough to also see the shading.
    Camera camera(900, 650, glm::vec3(6.0f, 2.5f, FINISH_Z + 9.0f));

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.SetView(camera.Position, glm::vec3(0.0f, 0.0f, FINISH_Z), glm::vec3(0, 1, 0), 55.0f, 0.1f, 200.0f);
        GLCompat::beginFrame(shader, camera, glm::vec3(-0.4f, -1.0f, -0.3f), glm::vec3(1.0f, 0.98f, 0.92f));

        drawBoundaryLinesCompat();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
