// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#include "ReplayEngine.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <sstream>
#include <string>
#include <thread>

namespace {
constexpr float k_minimum_speed = 0.001f;
constexpr std::size_t k_cps_window = 100;
constexpr auto k_inventory_poll_delay = std::chrono::milliseconds(5);
constexpr auto k_idle_poll_delay = std::chrono::milliseconds(2);
constexpr auto k_cps_update_interval = std::chrono::milliseconds(250);
}

ReplayEngine::ReplayEngine(Dataset& dataset,
                           Input& input,
                           Platform& platform,
                           Statistics& statistics)
    : dataset_(dataset),
      input_(input),
      platform_(platform),
      statistics_(statistics) {
}

ReplayEngine::~ReplayEngine() {
    stop();
}

void ReplayEngine::start(const ReplayConfig& config) {
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        if (state_.status != ReplayStatus::stopped) {
            return;
        }
    }

    if (thread_.joinable()) {
        thread_.join();
    }

    if (dataset_.empty()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        config_ = config;
        state_ = RuntimeState{};
        state_.status = ReplayStatus::disarmed;
    }

    stop_requested_ = false;
    thread_ = std::thread(&ReplayEngine::worker, this);
}

void ReplayEngine::stop() {
    stop_requested_ = true;

    if (thread_.joinable()) {
        thread_.join();
    }
}

ReplaySnapshot ReplayEngine::snapshot() const {
    ReplaySnapshot snapshot;

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        snapshot.status = state_.status;
        snapshot.current_index = state_.current_index;
        snapshot.next_click_ms = state_.next_click_ms;
        snapshot.current_interval_ms = state_.current_interval_ms;
        snapshot.click_count = state_.click_count;
        snapshot.target_cps = state_.target_cps;
        snapshot.actual_cps = state_.actual_cps;
        snapshot.speed = config_.speed;
    }

    snapshot.dataset_size = dataset_.size();
    return snapshot;
}

long ReplayEngine::scaled_interval(std::size_t index) const {
    const long interval = dataset_.interval(index);
    const float speed = std::max(config_.speed, k_minimum_speed);

    return std::max<long>(1, static_cast<long>(std::llround(interval / speed)));
}

double ReplayEngine::target_cps(std::size_t index) const {
    const std::size_t total = dataset_.size();
    if (total == 0) {
        return 0.0;
    }

    const std::size_t count = std::min(k_cps_window, total);
    const float speed = std::max(config_.speed, k_minimum_speed);

    double total_interval = 0.0;
    for (std::size_t offset = 0; offset < count; ++offset) {
        const std::size_t position = (index + offset) % total;
        total_interval += dataset_.interval(position) / speed;
    }

    if (total_interval <= 0.0) {
        return 0.0;
    }

    return 1000.0 * static_cast<double>(count) / total_interval;
}

bool ReplayEngine::toggle_inventory(bool& was_down, bool& inventory_open) {
    const bool down = input_.key_down(config_.inventory_key);

    if (down && !was_down) {
        inventory_open = !inventory_open;
    }

    was_down = down;
    return inventory_open;
}

bool ReplayEngine::wait_until(TimePoint deadline,
                              bool& toggle_was_down,
                              bool& inventory_was_down,
                              bool& inventory_open) {
    while (!stop_requested_) {
        if (input_.key_just_pressed(config_.toggle_key, toggle_was_down)) {
            bool armed;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                const bool was_armed = state_.status != ReplayStatus::disarmed &&
                                       state_.status != ReplayStatus::stopped;
                armed = !was_armed;
                state_.status = armed ? ReplayStatus::paused : ReplayStatus::disarmed;
                state_.next_click_ms = 0;
            }

            statistics_.log(armed
                ? "Replay armed. Hold the left mouse button over javaw.exe."
                : "Replay disarmed.");
            return false;
        }

        if (toggle_inventory(inventory_was_down, inventory_open)) {
            std::lock_guard<std::mutex> lock(state_mutex_);
            state_.status = ReplayStatus::inventory_paused;
            state_.next_click_ms = 0;
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (state_.status == ReplayStatus::disarmed) {
                state_.next_click_ms = 0;
                return false;
            }
        }

        if (!input_.mouse_held() || !platform_.is_javaw_foreground()) {
            std::lock_guard<std::mutex> lock(state_mutex_);
            state_.status = ReplayStatus::paused;
            state_.next_click_ms = 0;
            return false;
        }

        const TimePoint now = Clock::now();
        if (now >= deadline) {
            std::lock_guard<std::mutex> lock(state_mutex_);
            state_.next_click_ms = 0;
            return true;
        }

        const auto remaining = deadline - now;
        const auto remaining_ms = std::chrono::duration<double, std::milli>(remaining).count();

        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            state_.next_click_ms = std::max<long long>(
                0, static_cast<long long>(std::ceil(remaining_ms)));
        }

        if (remaining > std::chrono::milliseconds(3)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            std::this_thread::yield();
        }
    }

    return false;
}

void ReplayEngine::worker() {
    bool toggle_was_down = false;
    bool inventory_was_down = false;
    bool inventory_open = false;
    bool have_deadline = false;
    TimePoint next_deadline = Clock::now();
    TimePoint last_cps_update = Clock::now();
    long long last_click_ms = 0;
    std::size_t current_index = 0;

    statistics_.log("Replay ready.");
    statistics_.log(std::string("Replay toggle: ") + Input::key_name(config_.toggle_key));
    statistics_.log("Hold the left mouse button.");
    statistics_.log("Clicks are sent only to a foreground javaw.exe window.");
    statistics_.log("Scheduler: absolute deadlines, DOWN-to-DOWN intervals.");

    while (!stop_requested_) {
        const std::size_t total = dataset_.size();
        if (total == 0) {
            break;
        }

        if (current_index >= total) {
            std::random_device rd;
            std::uint32_t seed = static_cast<std::uint32_t>(rd());
            if (seed == dataset_.seed()) {
                ++seed;
            }

            dataset_.shuffle(seed);
            current_index = 0;
            last_click_ms = 0;
            have_deadline = false;

            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                state_.current_index = 0;
            }

            std::ostringstream message;
            message << "Replay completed. Starting a new loop (seed " << seed << ").";
            statistics_.log(message.str());
        }

        if (input_.key_just_pressed(config_.toggle_key, toggle_was_down)) {
            bool armed;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                const bool was_armed = state_.status != ReplayStatus::disarmed;
                armed = !was_armed;
                state_.status = armed ? ReplayStatus::paused : ReplayStatus::disarmed;
                state_.next_click_ms = 0;
            }

            have_deadline = false;
            statistics_.log(armed ? "Replay armed. Hold the left mouse button over javaw.exe."
                                  : "Replay disarmed.");
            continue;
        }

        if (toggle_inventory(inventory_was_down, inventory_open)) {
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                state_.status = ReplayStatus::inventory_paused;
                state_.next_click_ms = 0;
            }
            have_deadline = false;
            std::this_thread::sleep_for(k_inventory_poll_delay);
            continue;
        }

        bool armed = false;
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            armed = state_.status != ReplayStatus::disarmed &&
                    state_.status != ReplayStatus::stopped;
        }

        const bool can_run = armed && input_.mouse_held() && platform_.is_javaw_foreground();
        if (!can_run) {
            bool was_active = false;
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                was_active = state_.status == ReplayStatus::active;
                if (state_.status != ReplayStatus::disarmed &&
                    state_.status != ReplayStatus::inventory_paused) {
                    state_.status = ReplayStatus::paused;
                }
                state_.next_click_ms = 0;
            }

            if (was_active) {
                statistics_.log("Replay paused: mouse released or javaw.exe is not foreground.");
            }

            have_deadline = false;
            std::this_thread::sleep_for(k_idle_poll_delay);
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            state_.status = ReplayStatus::active;
        }

        if (!have_deadline) {
            const long interval = scaled_interval(current_index);
            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                state_.current_interval_ms = interval;
                state_.next_click_ms = interval;
            }

            next_deadline = Clock::now() + std::chrono::milliseconds(interval);
            have_deadline = true;
        }

        if (!wait_until(next_deadline,
                        toggle_was_down,
                        inventory_was_down,
                        inventory_open)) {
            have_deadline = false;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (state_.status != ReplayStatus::active) {
                state_.next_click_ms = 0;
                have_deadline = false;
                continue;
            }
        }

        if (!input_.mouse_held() || !platform_.is_javaw_foreground()) {
            have_deadline = false;
            continue;
        }

        const long interval = scaled_interval(current_index);
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            state_.current_interval_ms = interval;
            state_.next_click_ms = 0;
        }

        long long click_time_ms;
        if (config_.dry_run) {
            click_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now().time_since_epoch()).count();
        } else {
            std::string error;
            click_time_ms = platform_.send_left_click(error);
            if (!error.empty()) {
                statistics_.log(error);
            }
        }

        if (last_click_ms > 0 && click_time_ms > last_click_ms) {
            statistics_.push_interval(static_cast<float>(click_time_ms - last_click_ms));
        }
        last_click_ms = click_time_ms;

        ++current_index;
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            state_.current_index = current_index;
            ++state_.click_count;
        }

        if (current_index < total) {
            next_deadline += std::chrono::milliseconds(scaled_interval(current_index));
            std::lock_guard<std::mutex> lock(state_mutex_);
            state_.current_interval_ms = scaled_interval(current_index);
        } else {
            have_deadline = false;
        }

        const TimePoint now = Clock::now();
        if (now - last_cps_update >= k_cps_update_interval) {
            const double actual = statistics_.recent_cps();
            const double target = target_cps(current_index);

            {
                std::lock_guard<std::mutex> lock(state_mutex_);
                state_.actual_cps = actual;
                state_.target_cps = target;
            }

            statistics_.set_actual_cps(actual);
            statistics_.set_target_cps(target);
            statistics_.push_cps(static_cast<float>(actual));
            last_cps_update = now;
        }
    }

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        state_.status = ReplayStatus::stopped;
        state_.next_click_ms = 0;
    }

    if (stop_requested_) {
        statistics_.log("Replay stopped.");
    }
}
