#include "GaussianData.h"

#include "Utils.h"

namespace gsplat {

void GaussianData::pack() {
    size_t n = positions.size();
    
    // Allocate packed data - 2 uvec4 per gaussian
    packedData.resize(n * 8); // 8 uint32 per gaussian (2 uvec4)
    worldPositions.resize(n * 3);
    
    for (size_t i = 0; i < n; i++) {
        const glm::vec3& pos = positions[i];
        const glm::vec3& scale = scales[i];
        const glm::quat& rot = rotations[i];
        const glm::u8vec4& color = colors[i];
        
        // First uvec4: position (xyz as floats reinterpreted as uint) + selection flag (w)
        // Safe bit copy without violating strict aliasing
        uint32_t ux, uy, uz;
        std::memcpy(&ux, &pos.x, sizeof(uint32_t));
        std::memcpy(&uy, &pos.y, sizeof(uint32_t));
        std::memcpy(&uz, &pos.z, sizeof(uint32_t));
        packedData[i * 8 + 0] = ux;
        packedData[i * 8 + 1] = uy;
        packedData[i * 8 + 2] = uz;
        packedData[i * 8 + 3] = 0; // selection flag (0 = not selected)
        
        worldPositions[i * 3 + 0] = pos.x;
        worldPositions[i * 3 + 1] = pos.y;
        worldPositions[i * 3 + 2] = pos.z;
        
        // Convert quaternion to matrix for covariance
        glm::mat3 rotMat = glm::mat3_cast(rot);
        glm::mat3 scaleMat(0.0f);
        scaleMat[0][0] = scale.x;
        scaleMat[1][1] = scale.y;
        scaleMat[2][2] = scale.z;
        
        glm::mat3 M = rotMat * scaleMat;
        
        // Compute 3D covariance (symmetric)
        float sigma[6];
        sigma[0] = M[0][0] * M[0][0] + M[1][0] * M[1][0] + M[2][0] * M[2][0]; // xx
        sigma[1] = M[0][0] * M[0][1] + M[1][0] * M[1][1] + M[2][0] * M[2][1]; // xy
        sigma[2] = M[0][0] * M[0][2] + M[1][0] * M[1][2] + M[2][0] * M[2][2]; // xz
        sigma[3] = M[0][1] * M[0][1] + M[1][1] * M[1][1] + M[2][1] * M[2][1]; // yy
        sigma[4] = M[0][1] * M[0][2] + M[1][1] * M[1][2] + M[2][1] * M[2][2]; // yz
        sigma[5] = M[0][2] * M[0][2] + M[1][2] * M[1][2] + M[2][2] * M[2][2]; // zz
        
        // Second uvec4: covariance (xyz as half2) + color (w as packed RGBA)
        // Pack covariance as half-precision floats: xy, xz|yy, yz|zz
        packedData[i * 8 + 4] = packHalf2x16(sigma[0], sigma[1]); // xx, xy
        packedData[i * 8 + 5] = packHalf2x16(sigma[2], sigma[3]); // xz, yy
        packedData[i * 8 + 6] = packHalf2x16(sigma[4], sigma[5]); // yz, zz
        
        // Pack color as RGBA in a single uint32
        uint32_t packedColor = color.r | (color.g << 8) | (color.b << 16) | (color.a << 24);
        packedData[i * 8 + 7] = packedColor;
    }
}

void GaussianData::clear() {
    positions.clear();
    scales.clear();
    rotations.clear();
    colors.clear();
    packedData.clear();
    worldPositions.clear();
}

} // namespace gsplat
