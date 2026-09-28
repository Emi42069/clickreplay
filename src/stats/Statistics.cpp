#include "Statistics.h"

#include <cstddef>

void Statistics::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    interval_history_.clear();
    cps_history_.clear();
    recent_intervals_.clear();
    target_cps_ = 0.0;
    actual_cps_ = 0.0;
}

void Statistics::push_interval(float value) {
    std::lock_guard<std::mutex> lock(mutex_);

    interval_history_.push_back(value);
    if (interval_history_.size() > k_history_size) {
        interval_history_.erase(interval_history_.begin());
    }

    recent_intervals_.push_back(value);
    if (recent_intervals_.size() > k_recent_interval_size) {
        recent_intervals_.erase(recent_intervals_.begin());
    }
}

void Statistics::push_cps(float value) {
    std::lock_guard<std::mutex> lock(mutex_);
    cps_history_.push_back(value);

    if (cps_history_.size() > k_history_size) {
        cps_history_.erase(cps_history_.begin());
    }
}

void Statistics::set_target_cps(double value) {
    std::lock_guard<std::mutex> lock(mutex_);
    target_cps_ = value;
}

void Statistics::set_actual_cps(double value) {
    std::lock_guard<std::mutex> lock(mutex_);
    actual_cps_ = value;
}

double Statistics::target_cps() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return target_cps_;
}

double Statistics::actual_cps() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return actual_cps_;
}

double Statistics::recent_cps() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (recent_intervals_.empty()) {
        return 0.0;
    }

    double total = 0.0;
    for (const float interval : recent_intervals_) {
        total += interval;
    }

    return total > 0.0
        ? 1000.0 * static_cast<double>(recent_intervals_.size()) / total
        : 0.0;
}

std::vector<float> Statistics::interval_history() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return interval_history_;
}

std::vector<float> Statistics::cps_history() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cps_history_;
}

void Statistics::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    logs_.push_back(message);

    if (logs_.size() > k_log_size) {
        logs_.erase(logs_.begin(), logs_.end() - static_cast<std::ptrdiff_t>(k_log_size));
    }
}

void Statistics::clear_logs() {
    std::lock_guard<std::mutex> lock(mutex_);
    logs_.clear();
}

std::vector<std::string> Statistics::logs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return logs_;
}
