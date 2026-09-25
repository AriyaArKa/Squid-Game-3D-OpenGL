#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "engine/Shader.h"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

// GLM-based view/projection camera (course tutorial 8, "Camera"). Exposes both the course's
// own free-fly WASD+mouse Inputs() (handy for engine debugging) and a SetView() the game's
// own camera-mode logic (third-person/Doll POV/etc., ported in a later milestone) can drive
// directly by giving it an explicit eye/target/up instead of flying it around by keyboard.
class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    int width, height;
    float speed = 0.06f;
    float sensitivity = 60.0f;
    bool firstClick = true;

    Camera(int width, int height, glm::vec3 position);

    // Recomputes view/projection from Position/Orientation/Up.
    void UpdateMatrix(float fovDeg, float nearPlane, float farPlane);
    // Recomputes view/projection from an explicit eye/target (used by the game's camera modes).
    void SetView(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up,
                 float fovDeg, float nearPlane, float farPlane);

    // Uploads view/projection/viewPos uniforms to the shader.
    void Apply(Shader& shader);

    void Inputs(GLFWwindow* window);
};

#endif // ENGINE_CAMERA_H
