// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#pragma once

#include <cstddef>
#include <mutex>
#include <string>
#include <vector>

class Statistics {
public:
    void reset();

    void push_interval(float value);
    void push_cps(float value);
    void set_target_cps(double value);
    void set_actual_cps(double value);

    double target_cps() const;
    double actual_cps() const;
    double recent_cps() const;

    std::vector<float> interval_history() const;
    std::vector<float> cps_history() const;

    void log(const std::string& message);
    void clear_logs();
    std::vector<std::string> logs() const;

private:
    static constexpr std::size_t k_history_size = 180;
    static constexpr std::size_t k_recent_interval_size = 100;
    static constexpr std::size_t k_log_size = 300;

    mutable std::mutex mutex_;
    std::vector<float> interval_history_;
    std::vector<float> cps_history_;
    std::vector<float> recent_intervals_;
    std::vector<std::string> logs_;
    double target_cps_ = 0.0;
    double actual_cps_ = 0.0;
};
