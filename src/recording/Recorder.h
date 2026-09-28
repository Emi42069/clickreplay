// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#pragma once

#include "RecordingSnapshot.h"

#include <chrono>
#include <cstddef>
#include <mutex>

class Dataset;
class Input;
class Statistics;

class Recorder {
public:
    Recorder(Dataset& dataset, Input& input, Statistics& statistics);
    ~Recorder();

    void start();
    void stop();
    RecordingSnapshot snapshot() const;

private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    void on_mouse_down();
    static long long elapsed_ms(TimePoint from, TimePoint to);

    Dataset& dataset_;
    Input& input_;
    Statistics& statistics_;

    mutable std::mutex mutex_;
    RecordingStatus status_ = RecordingStatus::stopped;
    std::size_t click_count_ = 0;
    long long last_interval_ms_ = 0;
    double cps_ = 0.0;
    TimePoint started_at_{};
    TimePoint last_click_at_{};
};
