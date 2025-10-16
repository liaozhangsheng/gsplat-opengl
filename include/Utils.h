#pragma once

#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstring>

namespace gsplat {

// Read entire file into string
inline std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Pack two 16-bit halfs into a 32-bit uint
inline uint16_t floatToHalf(float value) {
    // Avoid type-punning UB by using memcpy
    uint32_t f = 0;
    std::memcpy(&f, &value, sizeof(uint32_t));
    uint32_t sign = (f >> 31) & 0x0001;
    uint32_t exp = (f >> 23) & 0x00ff;
    uint32_t frac = f & 0x007fffff;
    
    uint32_t newExp;
    if (exp == 0) {
        newExp = 0;
    } else if (exp < 113) {
        newExp = 0;
        frac |= 0x00800000;
        frac >>= (113 - exp);
        if (frac & 0x01000000) {
            newExp = 1;
            frac = 0;
        }
    } else if (exp < 142) {
        newExp = exp - 112;
    } else {
        newExp = 31;
        frac = 0;
    }
    
    return (sign << 15) | (newExp << 10) | (frac >> 13);
}

inline uint32_t packHalf2x16(float x, float y) {
    uint16_t hx = floatToHalf(x);
    uint16_t hy = floatToHalf(y);
    return (uint32_t)hx | ((uint32_t)hy << 16);
}

// Spherical harmonics constant
constexpr float SH_C0 = 0.28209479177387814f;

// OpenGL error checking (defined in Renderer.cpp to avoid GL headers here)
void checkGLError(const char* context);

} // namespace gsplat
