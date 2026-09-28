#pragma once

#include <cstdint>

struct ReplayConfig {
    float speed = 1.0f;
    bool random_seed = true;
    std::uint32_t seed = 0;
    int toggle_key = 0x75;       // F6
    int inventory_key = 'R';
    bool dry_run = false;
};
