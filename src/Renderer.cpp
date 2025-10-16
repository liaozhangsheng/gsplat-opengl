#include <iostream>

#include "glm/gtc/type_ptr.hpp"

#include "Renderer.h"
#include "Utils.h"
#include "SplatSort.h"

namespace gsplat {

Renderer::Renderer(int width, int height)
    : width(width)
    , height(height)
    , program(0)
    , splatTexture(0)
    , vao(0)
    , positionVBO(0)
    , indexVBO(0)
    , splatCount(0)
    , textureWidth(0)
    , textureHeight(0)
{
    initShaders();
    initBuffers();
}

Renderer::~Renderer() {
    glDeleteProgram(program);
    glDeleteTextures(1, &splatTexture);
    glDeleteBuffers(1, &positionVBO);
    glDeleteBuffers(1, &indexVBO);
    glDeleteVertexArrays(1, &vao);
}

// Define GL error check here to keep Utils.h light
void checkGLError(const char* context) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in " << context << ": 0x" << std::hex << err << std::dec << std::endl;
    }
}

GLuint Renderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint Renderer::createProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }
    
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vertexShader);
    glAttachShader(prog, fragmentShader);
    glLinkProgram(prog);
    
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(prog, 512, nullptr, infoLog);
        std::cerr << "Program linking failed:\n" << infoLog << std::endl;
        glDeleteProgram(prog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return prog;
}

void Renderer::initShaders() {
    std::string vertexSource = readFile("shaders/splat.vert");
    std::string fragmentSource = readFile("shaders/splat.frag");
    
    program = createProgram(vertexSource.c_str(), fragmentSource.c_str());
    if (program == 0) {
        throw std::runtime_error("Failed to create shader program");
    }
    
    // Get uniform locations
    u_projection = glGetUniformLocation(program, "projection");
    u_view = glGetUniformLocation(program, "view");
    u_focal = glGetUniformLocation(program, "focal");
    u_viewport = glGetUniformLocation(program, "viewport");
    u_texture = glGetUniformLocation(program, "u_texture");
}

void Renderer::initBuffers() {
    // Create VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // Quad positions for point sprite
    float quadVertices[] = {
        -2.0f, -2.0f,
         2.0f, -2.0f,
         2.0f,  2.0f,
        -2.0f,  2.0f
    };
    
    glGenBuffers(1, &positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    a_position = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(a_position);
    glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    // Index buffer (per-instance)
    glGenBuffers(1, &indexVBO);
    
    glBindVertexArray(0);
    
    // Create textures
    glGenTextures(1, &splatTexture);
}

void Renderer::setGaussianData(const GaussianData& data) {
    gaussianData = data;
    splatCount = data.count();
    
    // Compute texture dimensions
    // Texture layout: each gaussian takes 2 columns (2 uvec4)
    // x coordinate: (index & 0x3ff) << 1 means 1024 gaussians per row
    // y coordinate: index >> 10
    textureWidth = 2048;  // 1024 * 2 columns
    textureHeight = std::max(1, (int)std::ceil(splatCount / 1024.0f));
    
    updateTextures();
}

void Renderer::updateTextures() {
    if (splatCount == 0) return;
    
    glUseProgram(program);
    
    // Reorganize packed data into 2D texture layout
    // Each gaussian occupies 2 horizontal pixels (columns)
    std::vector<uint32_t> textureData(textureWidth * textureHeight * 4);
    for (size_t i = 0; i < splatCount; i++) {
        int row = i / 1024;
        int col = (i % 1024) * 2;  // Each gaussian takes 2 columns
        
        // First uvec4 (position + selection)
        int idx1 = (row * textureWidth + col) * 4;
        textureData[idx1 + 0] = gaussianData.packedData[i * 8 + 0];
        textureData[idx1 + 1] = gaussianData.packedData[i * 8 + 1];
        textureData[idx1 + 2] = gaussianData.packedData[i * 8 + 2];
        textureData[idx1 + 3] = gaussianData.packedData[i * 8 + 3];
        
        // Second uvec4 (covariance + color)
        int idx2 = (row * textureWidth + col + 1) * 4;
        textureData[idx2 + 0] = gaussianData.packedData[i * 8 + 4];
        textureData[idx2 + 1] = gaussianData.packedData[i * 8 + 5];
        textureData[idx2 + 2] = gaussianData.packedData[i * 8 + 6];
        textureData[idx2 + 3] = gaussianData.packedData[i * 8 + 7];
    }
    
    // Upload splat data texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, splatTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, textureWidth, textureHeight, 0,
                 GL_RGBA_INTEGER, GL_UNSIGNED_INT, textureData.data());
    glUniform1i(u_texture, 0);
    checkGLError("Upload splat texture");
}

void Renderer::sortSplats(const glm::mat4& viewProj) {
    if (splatCount == 0) return;
    
    SplatSort::sort(viewProj, gaussianData.worldPositions.data(), splatCount, depthIndex);
}

void Renderer::render(Camera& camera) {
    if (splatCount == 0) {
        std::cerr << "Warning: splatCount is 0" << std::endl;
        return;
    }
    
    camera.update();
    
    // Sort splats
    sortSplats(camera.getViewProjMatrix());
    
    // Upload sorted indices
    glBindBuffer(GL_ARRAY_BUFFER, indexVBO);
    glBufferData(GL_ARRAY_BUFFER, depthIndex.size() * sizeof(uint32_t), 
                 depthIndex.data(), GL_STREAM_DRAW);
    checkGLError("Upload indices");
    
    // Setup OpenGL state
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // Zero alpha so front-to-back blending accumulates correctly
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    checkGLError("Setup blend state");
    checkGLError("Setup GL state");
    
    glUseProgram(program);
    glBindVertexArray(vao);
    
    // Set uniforms
    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
    glUniform2f(u_focal, camera.getFx(), camera.getFy());
    glUniform2f(u_viewport, static_cast<float>(width), static_cast<float>(height));
    checkGLError("Set uniforms");
    
    // Setup vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glEnableVertexAttribArray(a_position);
    glVertexAttribPointer(a_position, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, indexVBO);
    if (a_index == -1) {
        a_index = glGetAttribLocation(program, "index");
    }
    glEnableVertexAttribArray(a_index);
    glVertexAttribIPointer(a_index, 1, GL_UNSIGNED_INT, 0, 0);
    glVertexAttribDivisor(a_index, 1);
    checkGLError("Setup vertex attributes");
    
    // Draw
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, splatCount);
    checkGLError("Draw");
    
    glBindVertexArray(0);
}

void Renderer::resize(int w, int h) {
    width = w;
    height = h;
}

} // namespace gsplat
