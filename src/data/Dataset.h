#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

class Dataset {
public:
    bool load_csv(const std::string& path, std::string& error);

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
