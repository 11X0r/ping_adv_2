#pragma once
#include "stats/ping_stats.hpp"
#include <string>
#include <memory>
#include <chrono>

struct ping_rts;
struct socket_st;

class PingWrapper {
public:
    static constexpr const char* VERSION = "ping_adv (internal iputils)";
    
    explicit PingWrapper(const std::string& target_ip);
    ~PingWrapper();

    PingStats execute(int count, std::chrono::duration<double, std::milli> interval);
    
    static std::string get_version() { return VERSION; }

private:
    void cleanup();
    
    std::unique_ptr<ping_rts> rts_;
    std::unique_ptr<socket_st> sock_;
};