#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace gsplat {

struct GaussianData {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> scales;
    std::vector<glm::quat> rotations;
    std::vector<glm::u8vec4> colors;
    
    // Packed data for GPU
    std::vector<uint32_t> packedData;
    std::vector<float> worldPositions;  // Only needed for sorting
    
    size_t count() const { return positions.size(); }
    
    void pack();
    void clear();
};

} // namespace gsplat
