#include "ping_wrapper.hpp"
#undef _GNU_SOURCE
#include "ping_bridge.hpp"

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <algorithm>

extern "C" {
    void ping_setup(struct ping_rts* rts, struct socket_st* sock) {
        int hold = 1;
        setsockopt(sock->fd, SOL_IP, IP_TTL, &hold, sizeof(hold));
    }

    int ping_send_probe(struct ping_rts* rts, struct socket_st* sock, void* packet, unsigned int packet_size) {
        struct icmp *icp = (struct icmp*)packet;
        icp->icmp_type = ICMP_ECHO;
        icp->icmp_code = 0;
        icp->icmp_cksum = 0;
        icp->icmp_seq = 0;
        icp->icmp_id = rts->ident;
        
        return sendto(sock->fd, packet, packet_size, 0,
                     (struct sockaddr*)&rts->whereto,
                     sizeof(rts->whereto));
    }

    int ping_parse_reply(struct ping_rts* rts, struct socket_st* sock, struct msghdr* msg, int cc,
                        void* addr, struct timeval* tv) {
        struct icmp *icp;
        struct iphdr *ip = (struct iphdr *)msg->msg_iov->iov_base;
        
        icp = (struct icmp *)((char *)msg->msg_iov->iov_base + (ip->ihl << 2));
        if (icp->icmp_type == ICMP_ECHOREPLY && icp->icmp_id == rts->ident) {
            return 0;
        }
        return 1;
    }
}

PingWrapper::PingWrapper(const std::string& target_ip) {
    rts_ = std::make_unique<ping_rts>();
    sock_ = std::make_unique<socket_st>();
    
    std::memset(rts_.get(), 0, sizeof(ping_rts));
    std::memset(sock_.get(), 0, sizeof(socket_st));
    sock_->fd = -1;

    rts_->datalen = 56;
    rts_->timing = 1;
    rts_->opt_quiet = 1;

    rts_->whereto.sin_family = AF_INET;
    if (inet_pton(AF_INET, target_ip.c_str(), &rts_->whereto.sin_addr) <= 0) {
        throw std::runtime_error("Invalid IP address");
    }

    sock_->fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock_->fd < 0) {
        throw std::runtime_error("Failed to create socket (root required)");
    }
    sock_->socktype = SOCK_RAW;

    rts_->ident = getpid() & 0xFFFF;
    iputils::ping_setup(rts_.get(), sock_.get());
}

PingWrapper::~PingWrapper() {
    cleanup();
}

void PingWrapper::cleanup() {
    if (sock_ && sock_->fd >= 0) {
        close(sock_->fd);
        sock_->fd = -1;
    }
}

PingStats PingWrapper::execute(int count, std::chrono::duration<double, std::milli> interval) {
    if (count < 2 || count > 50) {
        throw std::invalid_argument("Count must be between 2 and 50");
    }
    
    const auto interval_ms = interval.count();
    if (interval_ms < 0.1 || interval_ms > 10.0) {
        throw std::invalid_argument("Interval must be between 0.1ms and 10ms");
    }

    PingStats stats;
    rts_->interval = std::chrono::duration_cast<std::chrono::microseconds>(interval).count();

    int failures = 0;
    const int max_failures = std::min(5, count / 2);

    for (int i = 0; i < count && failures < max_failures; ++i) {
        auto start_time = std::chrono::steady_clock::now();

        if (iputils::ping_send_probe(rts_.get(), sock_.get(), rts_->outpack, sizeof(rts_->outpack)) < 0) {
            failures++;
            continue;
        }

        char buffer[4096], cbuf[4096];
        struct msghdr msg{};
        struct iovec iov{};
        
        iov.iov_base = buffer;
        iov.iov_len = sizeof(buffer);
        msg.msg_name = &rts_->whereto;
        msg.msg_namelen = sizeof(rts_->whereto);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cbuf;
        msg.msg_controllen = sizeof(cbuf);

        struct timeval timeout = { 1, 0 };
        if (iputils::ping_parse_reply(rts_.get(), sock_.get(), &msg, 
            recvmsg(sock_->fd, &msg, 0), 
            &rts_->whereto, &timeout) == 0) {
            auto end_time = std::chrono::steady_clock::now();
            stats.add_result(std::chrono::duration_cast<std::chrono::nanoseconds>(
                end_time - start_time));
        } else {
            failures++;
        }

        if (i < count - 1) {
            std::this_thread::sleep_for(interval);
        }
    }

    return stats;
}