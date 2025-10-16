#pragma once

namespace gsplat {

class Renderer;
class OrbitControls;

struct AppContext {
    Renderer* renderer = nullptr;
    OrbitControls* controls = nullptr;
};

} // namespace gsplat
