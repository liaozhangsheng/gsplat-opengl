#pragma once

#include <vector>
#include <cstdint>

#include "glm/glm.hpp"

namespace gsplat {

class SplatSort {
public:
    static void sort(
        const glm::mat4& viewProj,
        const float* positions,
        uint32_t vertexCount,
        std::vector<uint32_t>& depthIndex
    );
};

} // namespace gsplat
