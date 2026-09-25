#include "engine/Camera.h"
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

Camera::Camera(int width, int height, glm::vec3 position) {
    this->width = width;
    this->height = height;
    Position = position;
}

void Camera::UpdateMatrix(float fovDeg, float nearPlane, float farPlane) {
    view = glm::lookAt(Position, Position + Orientation, Up);
    projection = glm::perspective(glm::radians(fovDeg), (float)width / (float)height, nearPlane, farPlane);
}

void Camera::SetView(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up,
                      float fovDeg, float nearPlane, float farPlane) {
    Position = eye;
    Orientation = glm::normalize(target - eye);
    Up = up;
    view = glm::lookAt(eye, target, up);
    projection = glm::perspective(glm::radians(fovDeg), (float)width / (float)height, nearPlane, farPlane);
}

void Camera::Apply(Shader& shader) {
    shader.Activate();
    shader.SetMat4("view", view);
    shader.SetMat4("projection", projection);
    shader.SetVec3("viewPos", Position);
}

void Camera::Inputs(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Position += speed * Orientation;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Position -= speed * Orientation;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Position -= speed * glm::normalize(glm::cross(Orientation, Up));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Position += speed * glm::normalize(glm::cross(Orientation, Up));
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        Position += speed * Up;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        Position -= speed * Up;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (firstClick) {
            glfwSetCursorPos(window, width / 2.0, height / 2.0);
            firstClick = false;
        }

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float rotX = sensitivity * (float)(mouseY - (height / 2.0)) / height;
        float rotY = sensitivity * (float)(mouseX - (width / 2.0)) / width;

        glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));
        if (glm::angle(newOrientation, Up) > glm::radians(5.0f) && glm::angle(newOrientation, -Up) > glm::radians(5.0f)) {
            Orientation = newOrientation;
        }
        Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

        glfwSetCursorPos(window, width / 2.0, height / 2.0);
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }
}
