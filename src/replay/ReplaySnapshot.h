#pragma once

#include <cstddef>

enum class ReplayStatus {
    stopped,
    disarmed,
    paused,
    active,
    inventory_paused
};

struct ReplaySnapshot {
    ReplayStatus status = ReplayStatus::stopped;
    std::size_t current_index = 0;
    std::size_t dataset_size = 0;

    long long next_click_ms = 0;
    long long current_interval_ms = 0;
    long long click_count = 0;

    double target_cps = 0.0;
    double actual_cps = 0.0;
    float speed = 1.0f;
};
