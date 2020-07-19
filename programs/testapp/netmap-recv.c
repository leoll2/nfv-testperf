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

#include "timestamp.h"

#include "newcommon.h"
#include "stats.h"


int netmap_recv_body(int argc, char *argv[])
{
    struct config conf = CONFIG_STRUCT_INITIALIZER;

    // Default configuration for local program
    conf.local_port = RECV_PORT;
    conf.remote_port = SEND_PORT;
    conf.socktype = SOCK_DGRAM;
    strcpy(conf.local_ip, RECV_ADDR_IP);
    strcpy(conf.remote_ip, SEND_ADDR_IP);
    strcpy(conf.local_mac, RECV_ADDR_MAC);
    strcpy(conf.remote_mac, SEND_ADDR_MAC);

    parameters_parse(argc, argv, &conf);

    print_config(&conf);

    signal(SIGINT, handle_sigint);

    /* TODO rework*/
    while(true)
        printf("[TODO] netmap-recv");

    // Should never get here actually
    close(conf.sock_fd);

    return EXIT_SUCCESS;
}
