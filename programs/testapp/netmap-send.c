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

#include "netmap/pkt-gen.h"


int netmap_send_body(int argc, char *argv[])
{
    int netmap_argc = 1;
    char **netmap_argv = NULL;
    struct config conf = CONFIG_STRUCT_INITIALIZER;

    // Default configuration for local program  /* TODO is this needed? */
    conf.local_port = SEND_PORT;
    conf.remote_port = RECV_PORT;
    conf.socktype = SOCK_DGRAM;
    strcpy(conf.local_ip, SEND_ADDR_IP);
    strcpy(conf.remote_ip, RECV_ADDR_IP);
    strcpy(conf.local_mac, SEND_ADDR_MAC);
    strcpy(conf.remote_mac, RECV_ADDR_MAC);

    parameters_parse(argc, argv, &conf);

    print_config(&conf);

    netmap_argv = malloc(sizeof(char *));
    netmap_argv[0] = "netmap_sender";

    /* Packet size */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-l";
    netmap_argv = malloc(16 * sizeof(char));   // 16 digits should be enough
    snprintf(netmap_argv[netmap_argc-1], 16, "%d", conf.pkt_size);

    /* Last arg must be NULL by convention */
    netmap_argv[netmap_argc] = NULL;

    netmap_main_loop(netmap_argc, netmap_argv);
        
    return EXIT_SUCCESS;
}
