#include <algorithm>
#include <vector>

#include "SplatSort.h"

namespace gsplat {

void SplatSort::sort(
    const glm::mat4& viewProj,
    const float* positions,
    uint32_t vertexCount,
    std::vector<uint32_t>& depthIndex
) {
    // Compute depths
    std::vector<std::pair<float, uint32_t>> depths(vertexCount);
    
    for (uint32_t i = 0; i < vertexCount; i++) {
        glm::vec3 pos(
            positions[i * 3 + 0],
            positions[i * 3 + 1],
            positions[i * 3 + 2]
        );
        
        glm::vec4 projected = viewProj * glm::vec4(pos, 1.0f);
        float depth = projected.z / projected.w;
        
        depths[i] = {depth, i};
    }
    
    // Sort by depth (front to back for weighted blended transparency)
    std::sort(depths.begin(), depths.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });
    
    // Extract sorted indices
    depthIndex.resize(vertexCount);
    for (uint32_t i = 0; i < vertexCount; i++) {
        depthIndex[i] = depths[i].second;
    }
}

} // namespace gsplat
