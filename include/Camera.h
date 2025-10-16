#pragma once

#include "glm/glm.hpp"

namespace gsplat {

class Camera {
public:
    Camera(int width, int height, float fov = 45.0f);
    
    void setSize(int width, int height);
    void setFov(float fov);
    void setPosition(const glm::vec3& pos);
    void setTarget(const glm::vec3& target);
    void setUp(const glm::vec3& up);
    
    void update();
    
    const glm::mat4& getViewMatrix() const { return viewMatrix; }
    const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }
    const glm::mat4& getViewProjMatrix() const { return viewProjMatrix; }
    
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getTarget() const { return target; }
    glm::vec3 getUp() const { return up; }
    
    float getFx() const { return fx; }
    float getFy() const { return fy; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 viewProjMatrix;
    
    int width, height;
    float fov;
    float fx, fy;
    float aspect;
    float nearPlane, farPlane;
};

} // namespace gsplat
