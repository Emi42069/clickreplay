// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#pragma once

#include "ReplayConfig.h"
#include "ReplaySnapshot.h"
#include "../data/Dataset.h"
#include "../input/Input.h"
#include "../platform/Platform.h"
#include "../stats/Statistics.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <thread>

class ReplayEngine {
public:
    ReplayEngine(Dataset& dataset,
                 Input& input,
                 Platform& platform,
                 Statistics& statistics);
    ~ReplayEngine();

    void start(const ReplayConfig& config);
    void stop();
    ReplaySnapshot snapshot() const;

private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    struct RuntimeState {
        ReplayStatus status = ReplayStatus::stopped;
        std::size_t current_index = 0;
        long long next_click_ms = 0;
        long long current_interval_ms = 0;
        long long click_count = 0;
        double target_cps = 0.0;
        double actual_cps = 0.0;
    };

    void worker();
    bool wait_until(TimePoint deadline,
                    bool& toggle_was_down,
                    bool& inventory_was_down,
                    bool& inventory_open);
    bool toggle_inventory(bool& was_down, bool& inventory_open);
    long scaled_interval(std::size_t index) const;
    double target_cps(std::size_t index) const;

    Dataset& dataset_;
    Input& input_;
    Platform& platform_;
    Statistics& statistics_;

    ReplayConfig config_{};
    RuntimeState state_{};

    std::thread thread_;
    std::atomic<bool> stop_requested_{false};
    mutable std::mutex state_mutex_;
};
