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

    // The starting point for packet payload, zero for UDP sockets
    const ssize_t base_offset = OFFSET_PAYLOAD;

    // ------------------- Variables and data structures -------------------- //

    // Data structures used to hold packets
    // This one is used for simple sockets send/recv
    byte_t pkt[used_len];

    // Timers and counters
    const tsc_t tsc_hz = tsc_get_hz();
    const tsc_t tsc_out = tsc_hz;
    tsc_t tsc_cur, tsc_prev;

    // Stats variables
    struct stats stats = STATS_INIT;
    stats.type = STATS_RX;
    stats_ptr = &stats;

    struct stats_data_rx stats_period = {0};

    // Netmap-related
    struct pollfd pfd;
    struct netmap_if *nifp;
    struct netmap_ring *ring;
    nmd = nm_open(conf->local_interf, NULL, 0, 0);
    pfd.fd = NETMAP_FD(nmd);
    pfd.events = POLLIN;
    nifp = nmd->nifp;

    // --------------------------- Initialization --------------------------- //

    // Initialize addrlen
    // addrlen = sizeof(src_addr);

    // Structures initialization
    memset(pkt, 0, sizeof(pkt));

    // ------------------ Wait for the first packet to arrive ------------------ //

    int pollret;

    for(;;)
    {
        // Check every second
        pollret = poll(&pfd, 1, 1000);
        if (pollret > 0 && !(pfd.revents & POLLERR))
			break;
		if (pollret < 0) {
			fprintf(stderr, "poll() error: %s", strerror(errno));
			quit_handler(SIGINT);
		}
		if (pfd.revents & POLLERR) {
			fprintf(stderr, "fd error");
			quit_handler(SIGINT);
		}
        fprintf(stdout, "Waiting for the first packet...\n");
    }

    // ------------------ Infinite loop variables and body ------------------ //

    tsc_cur = tsc_prev = tsc_get_update();

    for (;;)
    {
        tsc_cur = tsc_get_update();

        // If more than a second elapsed, print stats
        if (tsc_cur - tsc_prev > tsc_out)
        {
            // Save stats
            stats_save(&stats, (union stats_data *)&stats_period);

            // If not silent, print them
            if (!conf->silent)
            {
                stats_print(STATS_RX, (union stats_data *)&stats_period);
            }

            // Reset stats for the new period
            stats_period.rx = 0;

            // Update timers
            tsc_prev = tsc_cur;
        }
#ifdef BUSYWAIT /* Non-blocking API */
        if (ioctl(pfd.fd, NIOCRXSYNC, NULL) < 0) {
            fprintf(stderr, "ioctl NIOCRXSYNC failed\n");
            quit_handler(SIGINT);
        }
#else           /* Blocking API */
        pollret = poll(&pfd, 1, 1000);
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
        // For each ring, read a batch of packets
		for (int ri = nmd->first_rx_ring; ri <= nmd->last_rx_ring; ri++) {
            unsigned int head, to_read, avail_rx_slots, b_len;

			ring = NETMAP_RXRING(nifp, ri);
			if (nm_ring_empty(ring))
				continue;

            head = ring->head;
            avail_rx_slots = nm_ring_space(ring);
            if (conf->bst_size > avail_rx_slots)
                to_read = avail_rx_slots;
            else
                to_read = conf->bst_size;

            for (unsigned int si = 0; si < to_read; si++) {
                struct netmap_slot *slot = &ring->slot[head];
                char *b = NETMAP_BUF(ring, slot->buf_idx);
                b_len = slot->len;
                if (b_len != frame_len) {
                    fprintf(stderr, "wrong packet length (%u != %lu)\n", b_len, frame_len);
                }
                if (conf->touch_data) {
                    consume_data((unsigned char*)b + base_offset, payload_len);
                }
                head = nm_ring_next(ring, head);
            }
            ring->head = ring->cur = head;

            stats_period.rx += to_read;
		}
    }
    __builtin_unreachable();
}

int netmap_recv_body(int argc, char *argv[])
{
    struct config conf = CONFIG_STRUCT_INITIALIZER;

    // Default configuration for local program
    conf.local_port = RECV_PORT;
    conf.remote_port = SEND_PORT;
    strcpy(conf.local_ip, RECV_ADDR_IP);
    strcpy(conf.remote_ip, SEND_ADDR_IP);
    strcpy(conf.local_mac, RECV_ADDR_MAC);
    strcpy(conf.remote_mac, SEND_ADDR_MAC);

    parameters_parse(argc, argv, &conf);

    print_config(&conf);

    signal(SIGINT, quit_handler);

    tsc_init();

    main_loop(&conf);

    // Should never get here actually

    return EXIT_SUCCESS;
}
