// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#include "Recorder.h"

#include "../data/Dataset.h"
#include "../input/Input.h"
#include "../stats/Statistics.h"

#include <algorithm>

Recorder::Recorder(Dataset& dataset, Input& input, Statistics& statistics)
    : dataset_(dataset), input_(input), statistics_(statistics) {
    input_.set_mouse_down_callback([this]() { on_mouse_down(); });
}

Recorder::~Recorder() {
    input_.set_mouse_down_callback({});
}

void Recorder::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_ == RecordingStatus::recording) {
        return;
    }

    dataset_.clear();
    statistics_.reset();

    status_ = RecordingStatus::recording;
    click_count_ = 0;
    last_interval_ms_ = 0;
    cps_ = 0.0;
    started_at_ = Clock::now();
    last_click_at_ = TimePoint{};

    statistics_.log("Recording started. Click the left mouse button to build a dataset.");
}

void Recorder::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_ == RecordingStatus::stopped) {
        return;
    }

    status_ = RecordingStatus::stopped;
    statistics_.log("Recording stopped.");
}

RecordingSnapshot Recorder::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);

    RecordingSnapshot snapshot;
    snapshot.status = status_;
    snapshot.click_count = click_count_;
    snapshot.interval_count = dataset_.size();
    snapshot.last_interval_ms = last_interval_ms_;
    snapshot.cps = cps_;

    if (click_count_ > 0) {
        const TimePoint end = status_ == RecordingStatus::recording
            ? Clock::now()
            : last_click_at_;
        snapshot.duration_ms = elapsed_ms(started_at_, end);
    }

    return snapshot;
}

void Recorder::on_mouse_down() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_ != RecordingStatus::recording) {
        return;
    }

    const TimePoint now = Clock::now();
    ++click_count_;

    if (last_click_at_ != TimePoint{}) {
        const long long interval = std::max(1LL, elapsed_ms(last_click_at_, now));
        dataset_.append_interval(interval);
        statistics_.push_interval(static_cast<float>(interval));
        last_interval_ms_ = interval;
        cps_ = statistics_.recent_cps();
        statistics_.set_actual_cps(cps_);
    }

    last_click_at_ = now;
}

long long Recorder::elapsed_ms(TimePoint from, TimePoint to) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(to - from).count();
}
