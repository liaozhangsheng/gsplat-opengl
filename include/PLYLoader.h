#pragma once

#include <string>

#include "GaussianData.h"

namespace gsplat {

class PLYLoader {
public:
    static GaussianData load(const std::string& path);
};

} // namespace gsplat
