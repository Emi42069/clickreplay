// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

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
