#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>

#define NETMAP_WITH_LIBS
#include <net/netmap_user.h>

#include "timestamp.h"

#include "newcommon.h"
#include "stats.h"

struct nm_desc *nmd;        // netmap descriptor, global to close it easily


// main loop function shall not return because the signal handler for SIGINT
// terminates the program non-gracefully
static inline void main_loop(struct config *conf) __attribute__((noreturn));


static void quit_handler(int sig)
{
    nm_close(nmd);

    handle_sigint(sig);
}


static inline void main_loop(struct config *conf)
{
    // ----------------------------- Constants ------------------------------ //

    const ssize_t frame_len = conf->pkt_size - 4;
    const ssize_t payload_len = frame_len - OFFSET_PAYLOAD;

    // The actual size of buffers that need to be allocated
    const ssize_t used_len = frame_len;

    // The starting point for packet payload
    const ssize_t base_offset = OFFSET_PAYLOAD;

    // ------------------- Variables and data structures -------------------- //

    // Header structures
    struct ether_hdr pkt_eth_hdr;
    struct ipv4_hdr pkt_ip_hdr;
    struct udp_hdr pkt_udp_hdr;

    // Data structure used to hold the frame header
    byte_t frame_hdr[PKT_HEADER_SIZE];

    // Data structures used to hold packets
    // This one is used for simple sockets send/recv
    byte_t pkt[used_len];

    // Timers and counters
    const tsc_t tsc_hz = tsc_get_hz();
    const tsc_t tsc_out = tsc_hz;
    const tsc_t tsc_incr = tsc_hz * conf->bst_size / conf->rate;
    tsc_t tsc_cur, tsc_prev, tsc_next;

    // Stats variables
    struct stats stats = STATS_INIT;
    stats.type = STATS_TX;
    stats_ptr = &stats;

    struct stats_data_tx stats_period = {0, 0};

    // Netmap-related
    struct pollfd pfd;
    struct netmap_if *nifp;
    struct netmap_ring *ring;
    struct netmap_slot *slot = NULL;
    nmd = nm_open(conf->local_interf, NULL, 0, 0);
    pfd.fd = NETMAP_FD(nmd);
    pfd.events = POLLOUT;
    nifp = nmd->nifp;


    // --------------------------- Initialization --------------------------- //

    // Structures initialization
    memset(frame_hdr, 0, sizeof(frame_hdr));
    memset(pkt, 0, sizeof(pkt));

    // Build a frame header
    dpdk_setup_pkt_headers(&pkt_eth_hdr, &pkt_ip_hdr, &pkt_udp_hdr, conf);
    memcpy(frame_hdr + OFFSET_ETHER, &pkt_eth_hdr, sizeof(struct ether_hdr));
    memcpy(frame_hdr + OFFSET_IPV4, &pkt_ip_hdr, sizeof(struct ipv4_hdr));
    memcpy(frame_hdr + OFFSET_UDP, &pkt_udp_hdr, sizeof(struct udp_hdr));

    // ------------------ Infinite loop variables and body ------------------ //

    tsc_cur = tsc_prev = tsc_next = tsc_get_update();

    for (;;)
    {
        tsc_cur = tsc_get_update();

        // If more than a second elapsed
        if (tsc_cur - tsc_prev > tsc_out)
        {
            // Save stats
            stats_save(&stats, (union stats_data *)&stats_period);

            // If not silent, print them
            if (!conf->silent)
            {
                stats_print(STATS_TX, (union stats_data *)&stats_period);
            }

            // Reset stats for the new period
            stats_period.tx = 0;
            stats_period.dropped = 0;

            // Update timers
            tsc_prev = tsc_cur;
        }

        ring = NETMAP_TXRING(nifp, nmd->first_tx_ring);

        // If it is already time for the next burst (or in general tsc_next is
        // less than tsc_cur), send the next burst
        if (tsc_cur > tsc_next)
        {
            tsc_next += tsc_incr;

#ifdef BUSYWAIT /* Non-blocking API */
            if (ioctl(pfd.fd, NIOCTXSYNC, NULL) < 0) {
                fprintf(stderr, "ioctl NIOCTXSYNC failed\n");
                quit_handler(SIGINT);
            }
#else           /* Blocking API */
            int pollret = poll(&pfd, 1, 1000);
            if (pollret == 0) {
                fprintf(stderr, "Poll timeout, closing\n");
                quit_handler(SIGINT);
            } else if (pollret < 0) {
                fprintf(stderr, "poll() error: %s\n", strerror(errno));
                quit_handler(SIGINT);
            }
            if (pfd.revents & POLLERR) {
                fprintf(stderr, "fd error\n");
                quit_handler(SIGINT);
            }
#endif
            unsigned int bst_left = conf->bst_size;

            // For each ring
            for (int ri = nmd->first_tx_ring; ri <= nmd->last_tx_ring; ri++) {
                unsigned int head, avail_tx_slots, to_send;

                if (bst_left == 0)
                    break;

                ring = NETMAP_TXRING(nifp, ri);
                if (nm_ring_empty(ring))
                    continue;

                head = ring->head;
                avail_tx_slots = nm_ring_space(ring);
                if (bst_left > avail_tx_slots) {
                    to_send = avail_tx_slots;
                } else {
                    to_send = bst_left;
                }

                for (unsigned int si = 0; si < to_send; si++) {
                    slot = &ring->slot[head];
                    char *b = NETMAP_BUF(ring, slot->buf_idx);
                    memcpy((void *)b, frame_hdr, PKT_HEADER_SIZE);
                    produce_data((unsigned char *)b + base_offset, payload_len);
                    slot->len = frame_len;
                    head = nm_ring_next(ring, head);
                }
                slot->flags |= NS_REPORT;
                ring->head = ring->cur = head;
                if (to_send < bst_left) {
                    ring->cur = ring->tail;
                }

                bst_left -= to_send;
                stats_period.tx += to_send;
            }
        }
    }

    __builtin_unreachable();
}

int netmap_send_body(int argc, char *argv[])
{
    struct config conf = CONFIG_STRUCT_INITIALIZER;

    // Default configuration for local program
    conf.local_port = SEND_PORT;
    conf.remote_port = RECV_PORT;
    conf.socktype = SOCK_DGRAM;
    strcpy(conf.local_ip, SEND_ADDR_IP);
    strcpy(conf.remote_ip, RECV_ADDR_IP);
    strcpy(conf.local_mac, SEND_ADDR_MAC);
    strcpy(conf.remote_mac, RECV_ADDR_MAC);

    parameters_parse(argc, argv, &conf);

    fprintf(stdout, "[DBG] Local interface: %s\n", conf.local_interf);
    get_mac_from_iface(conf.local_interf + 7, conf.local_mac);

    print_config(&conf);

    signal(SIGINT, quit_handler);

    tsc_init();

    main_loop(&conf);

    // Should never get here actually
    close(conf.sock_fd);

    return EXIT_SUCCESS;
}
