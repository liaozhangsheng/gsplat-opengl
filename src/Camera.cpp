#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

namespace gsplat {

Camera::Camera(int width, int height, float fov)
    : position(0.0f, 5.0f, 5.0f)
    , target(0.0f, 0.0f, 0.0f)
    , up(0.0f, -1.0f, 0.0f)
    , width(width)
    , height(height)
    , fov(fov)
    , nearPlane(0.1f)
    , farPlane(100.0f)
{
    setSize(width, height);
    update();
}

void Camera::setSize(int w, int h) {
    width = w;
    height = h;
    aspect = static_cast<float>(width) / static_cast<float>(height);
    
    // Compute focal lengths from FOV
    float fovRad = glm::radians(fov);
    fy = height / (2.0f * std::tan(fovRad / 2.0f));
    fx = fy; // Assume square pixels
    
    projectionMatrix = glm::perspective(fovRad, aspect, nearPlane, farPlane);
}

void Camera::setFov(float newFov) {
    fov = newFov;
    setSize(width, height);
}

void Camera::setPosition(const glm::vec3& pos) {
    position = pos;
}

void Camera::setTarget(const glm::vec3& t) {
    target = t;
}

void Camera::setUp(const glm::vec3& u) {
    up = u;
}

void Camera::update() {
    viewMatrix = glm::lookAt(position, target, up);
    viewProjMatrix = projectionMatrix * viewMatrix;
}

} // namespace gsplat
