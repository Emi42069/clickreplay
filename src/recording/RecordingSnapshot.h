// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#pragma once

#include <cstddef>

enum class RecordingStatus {
    stopped,
    recording
};

struct RecordingSnapshot {
    RecordingStatus status = RecordingStatus::stopped;
    std::size_t click_count = 0;
    std::size_t interval_count = 0;
    long long duration_ms = 0;
    long long last_interval_ms = 0;
    double cps = 0.0;
};
