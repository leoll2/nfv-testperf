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

int netmap_server_body(int argc, char *argv[])
{
    int netmap_argc = 1;
    char **netmap_argv = NULL;
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

    netmap_argv = malloc(sizeof(char *));

    netmap_argv[0] = "netmap_pong";

    /* Function */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-f";
    netmap_argv[netmap_argc-1] = "pong";

    /* Interface */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-i";
    netmap_argv[netmap_argc-1] = malloc(32 * sizeof(char)); // 32 chars should be enough
    snprintf(netmap_argv[netmap_argc-1], 32, "%s", conf.local_interf);  // include netmap: prefix

    /* Sender MAC */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-S";
    netmap_argv[netmap_argc-1] = malloc(18 * sizeof(char));
    snprintf(netmap_argv[netmap_argc-1], 18, "%s", conf.local_mac);

    /* Destination MAC */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-D";
    netmap_argv[netmap_argc-1] = malloc(18 * sizeof(char));
    snprintf(netmap_argv[netmap_argc-1], 18, "%s", conf.remote_mac);

    /* Consume */
    if (conf.touch_data) {
        netmap_argc += 1;
        netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
        netmap_argv[netmap_argc-1] = "-e";
    }

    /* Dump */
    if (!conf.silent) {
        netmap_argc += 1;
        netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
        netmap_argv[netmap_argc-1] = "-X";
    }

    /* Last arg must be NULL by convention */
    netmap_argv[netmap_argc] = NULL;

    /* Print a list of the arguments: */
    printf("Calling pkt-gen with arguments: ");
    for (int i = 0; i < netmap_argc; i++) {
        printf("%s ", netmap_argv[i]);
    }
    printf("\n");

    printf("Launching netmap_main_loop on receiver\n");
    netmap_main_loop(netmap_argc, netmap_argv);
    printf("Finished netmap_main_loop on receiver\n");

    return EXIT_SUCCESS;
}
