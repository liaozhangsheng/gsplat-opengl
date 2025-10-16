#include "GLFW/glfw3.h"

#include "OrbitControls.h"
#include "AppContext.h"

namespace gsplat {

// Global callback wrappers
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    auto ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (ctx && ctx->controls) ctx->controls->handleMouseButton(button, action, mods);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    auto ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (ctx && ctx->controls) ctx->controls->handleMouseMove(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    auto ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (ctx && ctx->controls) ctx->controls->handleScroll(xoffset, yoffset);
}

OrbitControls::OrbitControls(GLFWwindow* window, Camera* camera)
    : window(window)
    , camera(camera)
    , lastX(0.0)
    , lastY(0.0)
    , rotating(false)
    , panning(false)
    , distance(5.0f)
    , theta(0.0f)
    , phi(M_PI / 4.0f)
    , targetPos(0.0f, 0.0f, 0.0f)
    , rotationSpeed(0.005f)
    , zoomSpeed(0.1f)
    , panSpeed(0.001f)
{
    // Note: glfwSetWindowUserPointer will be set in main.cpp with AppContext
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

void OrbitControls::update([[maybe_unused]] float deltaTime) {
    // Compute camera position from spherical coordinates
    float x = distance * std::sin(phi) * std::cos(theta);
    float y = distance * std::cos(phi);
    float z = distance * std::sin(phi) * std::sin(theta);
    
    camera->setPosition(targetPos + glm::vec3(x, y, z));
    camera->setTarget(targetPos);
}

void OrbitControls::reset() {
    distance = 5.0f;
    theta = 0.0f;
    phi = M_PI / 4.0f;
    targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
}

void OrbitControls::handleMouseButton(int button, int action, [[maybe_unused]] int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            rotating = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            rotating = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE || button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            panning = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            panning = false;
        }
    }
}

void OrbitControls::handleMouseMove(double xpos, double ypos) {
    double dx = xpos - lastX;
    double dy = ypos - lastY;
    
    if (rotating) {
        theta -= dx * rotationSpeed;
        phi -= dy * rotationSpeed;
        
        // Clamp phi to avoid gimbal lock
        phi = std::max(0.01f, std::min((float)M_PI - 0.01f, phi));
    } else if (panning) {
        // Pan in camera space
        glm::vec3 camPos = camera->getPosition();
        glm::vec3 forward = glm::normalize(targetPos - camPos);
        glm::vec3 right = glm::normalize(glm::cross(forward, camera->getUp()));
        glm::vec3 up = glm::normalize(glm::cross(right, forward));
        
        float panX = -dx * panSpeed * distance;
        float panY = dy * panSpeed * distance;
        
        targetPos += right * panX + up * panY;
    }
    
    lastX = xpos;
    lastY = ypos;
}

void OrbitControls::handleScroll([[maybe_unused]] double xoffset, double yoffset) {
    distance -= yoffset * zoomSpeed * distance;
    distance = std::max(0.1f, std::min(100.0f, distance));
}

} // namespace gsplat
