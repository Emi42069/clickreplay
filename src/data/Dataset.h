// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

class Dataset {
public:
    bool load_csv(const std::string& path, std::string& error);
    bool save_csv(const std::string& path, std::string& error) const;
    void clear();
    void append_interval(long interval);

    bool empty() const;
    std::size_t size() const;
    long interval(std::size_t index) const;
    std::vector<long> values() const;

    void shuffle(std::uint32_t seed);
    std::uint32_t seed() const;
    double cps() const;

private:
    mutable std::mutex mutex_;
    std::vector<long> intervals_;
    std::uint32_t seed_ = 0;
};
