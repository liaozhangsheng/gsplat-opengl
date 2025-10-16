#pragma once

#include <vector>

#include "glad/glad.h"

#include "Camera.h"
#include "GaussianData.h"

namespace gsplat {

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();
    
    void setGaussianData(const GaussianData& data);
    void render(Camera& camera);
    void resize(int width, int height);
    
    size_t getSplatCount() const { return splatCount; }

private:
    void initShaders();
    void initBuffers();
    void updateTextures();
    void sortSplats(const glm::mat4& viewProj);
    
    GLuint compileShader(GLenum type, const char* source);
    GLuint createProgram(const char* vertexSource, const char* fragmentSource);
    
    int width, height;
    
    // Shader program
    GLuint program;
    GLint u_projection, u_view, u_focal, u_viewport;
    GLint u_texture;
    
    // Textures
    GLuint splatTexture;
    
    // Buffers
    GLuint vao;
    GLuint positionVBO;
    GLuint indexVBO;
    // Cached attribute locations
    GLint a_position = -1;
    GLint a_index = -1;
    
    // Gaussian data
    GaussianData gaussianData;
    std::vector<uint32_t> depthIndex;
    size_t splatCount;
    
    // Texture dimensions
    int textureWidth, textureHeight;
};

} // namespace gsplat
