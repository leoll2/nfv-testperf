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


int netmap_clientst_body(int argc, char *argv[])
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

    printf("PING\n");   // TODO DEBUG
    parameters_parse(argc, argv, &conf);

    print_config(&conf);

    netmap_argv = malloc(sizeof(char *));

    netmap_argv[0] = "netmap_ping";

    /* Function */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-f";
    netmap_argv[netmap_argc-1] = "ping";
    
    /* Interface */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-i";
    netmap_argv[netmap_argc-1] = "netmap:veth_0_guest"; // TODO this must be parametric!

    /* Packet size */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-l";
    netmap_argv[netmap_argc-1] = malloc(16 * sizeof(char));   // 16 digits should be enough
    snprintf(netmap_argv[netmap_argc-1], 16, "%d", conf.pkt_size);

    /* Burst size */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-b";
    netmap_argv[netmap_argc-1] = malloc(16 * sizeof(char));   // 16 digits should be enough
    snprintf(netmap_argv[netmap_argc-1], 16, "%d", conf.bst_size);

    /* Rate */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-R";
    netmap_argv[netmap_argc-1] = malloc(16 * sizeof(char));   // 16 digits should be enough
    snprintf(netmap_argv[netmap_argc-1], 16, "%ld", conf.rate);
    
    /* Count */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-n";
    netmap_argv[netmap_argc-1] = malloc(32 * sizeof(char));   // 32 digits should be enough
    snprintf(netmap_argv[netmap_argc-1], 32, "%ld", conf.rate * 60); // TODO retrieve timeout somewhere from the configuration

    /* Last arg must be NULL by convention */
    netmap_argv[netmap_argc] = NULL;

    netmap_main_loop(netmap_argc, netmap_argv);
        
    return EXIT_SUCCESS;
}
