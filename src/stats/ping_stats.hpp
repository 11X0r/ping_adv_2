#pragma once
#include <chrono>
#include <vector>
#include <cstdint>

class PingStats {
public:
    void add_result(std::chrono::nanoseconds latency);

    bool is_valid() const { return latencies_.size() >= 2; }

    size_t count() const { return latencies_.size(); }

    double average_latency_ms() const;

    double jitter_ms() const;

private:
    std::vector<std::chrono::nanoseconds> latencies_;
};