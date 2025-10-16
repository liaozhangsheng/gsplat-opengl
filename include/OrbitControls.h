#pragma once

#include "Camera.h"

struct GLFWwindow;

namespace gsplat {

class OrbitControls {
public:
    OrbitControls(GLFWwindow* window, Camera* camera);
    
    void update(float deltaTime);
    void reset();
    
    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    void setZoomSpeed(float speed) { zoomSpeed = speed; }
    void setPanSpeed(float speed) { panSpeed = speed; }
    
    void handleMouseButton(int button, int action, int mods);
    void handleMouseMove(double xpos, double ypos);
    void handleScroll(double xoffset, double yoffset);
  
private:  

    GLFWwindow* window;
    Camera* camera;
    
    double lastX, lastY;
    bool rotating;
    bool panning;
    
    float distance;
    float theta, phi;
    glm::vec3 targetPos;
    
    float rotationSpeed;
    float zoomSpeed;
    float panSpeed;
    
    friend void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    friend void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    friend void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};

} // namespace gsplat
