#pragma once
#include "ping/ping.h"

namespace iputils {
extern "C" {
void ping_setup(struct ping_rts *rts, struct socket_st *sock);
int ping_send_probe(struct ping_rts *rts, struct socket_st *sock, void *packet,
                    unsigned int packet_size);
int ping_parse_reply(struct ping_rts *rts, struct socket_st *sock,
                     struct msghdr *msg, int cc, void *addr,
                     struct timeval *tv);
}
} // namespace iputils