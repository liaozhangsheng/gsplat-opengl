#include <iostream>
#include <chrono>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Renderer.h"
#include "Camera.h"
#include "PLYLoader.h"
#include "OrbitControls.h"
#include "AppContext.h"

using namespace gsplat;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    auto ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (ctx && ctx->renderer) {
        ctx->renderer->resize(width, height);
    }
}

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " <ply_file>\n";
    std::cout << "\nControls:\n";
    std::cout << "  Left Mouse:   Rotate camera\n";
    std::cout << "  Middle/Right: Pan camera\n";
    std::cout << "  Scroll:       Zoom in/out\n";
    std::cout << "  ESC:          Quit\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string plyPath = argv[1];
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    int width = 1280;
    int height = 720;
    GLFWwindow* window = glfwCreateWindow(width, height, "Gaussian Splat Viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    try {
        // Load PLY file
        std::cout << "Loading " << plyPath << "..." << std::endl;
        auto startLoad = std::chrono::high_resolution_clock::now();
        
        GaussianData data = PLYLoader::load(plyPath);
        
        auto endLoad = std::chrono::high_resolution_clock::now();
        auto loadTime = std::chrono::duration_cast<std::chrono::milliseconds>(endLoad - startLoad).count();
        
        std::cout << "Loaded " << data.count() << " Gaussians in " << loadTime << "ms" << std::endl;
        
        // Calculate bounding box for camera positioning
        glm::vec3 minPos(FLT_MAX), maxPos(-FLT_MAX);
        for (auto position: data.positions) {
            minPos = glm::min(minPos, position);
            maxPos = glm::max(maxPos, position);
        }
        glm::vec3 center = (minPos + maxPos) * 0.5f;
        glm::vec3 size = maxPos - minPos;
        float maxDim = std::max(std::max(size.x, size.y), size.z);
        float distance = std::max(maxDim * 2.0f, 1.0f);

        Renderer renderer(width, height);
        renderer.setGaussianData(data);

        Camera camera(width, height, 45.0f);
        camera.setPosition(center + glm::vec3(0.0f, 0.0f, distance));
        camera.setTarget(center);
        
        // Create controls
        OrbitControls controls(window, &camera);
        
        // Setup context for callbacks
        AppContext ctx;
        ctx.renderer = &renderer;
        ctx.controls = &controls;
        glfwSetWindowUserPointer(window, &ctx);
        
        std::cout << "\nRendering started. Press ESC to quit.\n" << std::endl;
        
        // Render loop
        auto lastFrame = std::chrono::high_resolution_clock::now();
        int frameCount = 0;
        double fpsTimer = 0.0;
        
        while (!glfwWindowShouldClose(window)) {
            auto currentFrame = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentFrame - lastFrame).count();
            lastFrame = currentFrame;
            
            // Update FPS counter
            frameCount++;
            fpsTimer += deltaTime;
            if (fpsTimer >= 1.0) {
                std::string title = "Gaussian Splat Viewer - " + std::to_string(frameCount) + " FPS - " +
                                    std::to_string(data.count()) + " Gaussians";
                glfwSetWindowTitle(window, title.c_str());
                frameCount = 0;
                fpsTimer = 0.0;
            }
            
            // Handle input
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }
            
            // Update controls and camera
            controls.update(deltaTime);
            
            // Render
            renderer.render(camera);
            
            // Swap buffers and poll events
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwTerminate();
    return 0;
}
