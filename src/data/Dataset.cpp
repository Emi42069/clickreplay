#include "Dataset.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <random>
#include <sstream>
#include <utility>

bool Dataset::load_csv(const std::string& path, std::string& error) {
    std::ifstream file(path);
    if (!file) {
        error = "Failed to open file: " + path;
        return false;
    }

    std::vector<long> intervals;
    std::string line;
    bool first_line = true;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        if (first_line) {
            first_line = false;
            const bool has_letters =
                line.find_first_of("abcdefghijklmnopqrstuvwxyz"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos;
            if (has_letters) {
                continue;
            }
        }

        std::stringstream row(line);
        std::string field;
        std::vector<std::string> fields;

        while (std::getline(row, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() < 3) {
            continue;
        }

        try {
            const long value = std::stol(fields[2]);
            if (value > 0) {
                intervals.push_back(value);
            }
        } catch (const std::exception&) {
            // Ignore malformed rows, as the original loader did.
        }
    }

    if (intervals.empty()) {
        error = "No valid interval found in CSV column 3.";
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        intervals_ = std::move(intervals);
        seed_ = 0;
    }

    error.clear();
    return true;
}

bool Dataset::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return intervals_.empty();
}

std::size_t Dataset::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return intervals_.size();
}

long Dataset::interval(std::size_t index) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return intervals_.at(index);
}

std::vector<long> Dataset::values() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return intervals_;
}

void Dataset::shuffle(std::uint32_t seed) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::mt19937 generator(seed);
    std::shuffle(intervals_.begin(), intervals_.end(), generator);
    seed_ = seed;
}

std::uint32_t Dataset::seed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return seed_;
}

double Dataset::cps() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (intervals_.size() < 2) {
        return 0.0;
    }

    double total = 0.0;
    for (std::size_t i = 1; i < intervals_.size(); ++i) {
        total += intervals_[i];
    }

    return total > 0.0
        ? 1000.0 * static_cast<double>(intervals_.size() - 1) / total
        : 0.0;
}
