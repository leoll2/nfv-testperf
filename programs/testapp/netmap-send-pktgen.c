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
    //char *iface_name;

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

    /* Function */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-f";
    netmap_argv[netmap_argc-1] = "tx";

    /* Consider the whole packet size for bps statistics */
    netmap_argc += 1;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-1] = "-B";
    
    /* Interface */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-i";
    netmap_argv[netmap_argc-1] = malloc(32 * sizeof(char)); // 32 chars should be enough
    snprintf(netmap_argv[netmap_argc-1], 32, "%s", conf.local_interf);  // include netmap: prefix
    //iface_name = netmap_argv[netmap_argc-1] + 7;            // exclude netmap: prefix

    /* Packet size */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-l";
    netmap_argv[netmap_argc-1] = malloc(16 * sizeof(char));   // 16 digits should be enough
    snprintf(netmap_argv[netmap_argc-1], 16, "%d", conf.pkt_size-4);    // length w/o CRC

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
    /*
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-n";
    netmap_argv[netmap_argc-1] = malloc(32 * sizeof(char));   // 32 digits should be enough
    snprintf(netmap_argv[netmap_argc-1], 32, "%ld", conf.rate * 60); // 60 seconds
    */

    /* Sender MAC */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-S";
    netmap_argv[netmap_argc-1] = malloc(18 * sizeof(char));
    //get_mac_from_iface(iface_name, netmap_argv[netmap_argc-1]);
    snprintf(netmap_argv[netmap_argc-1], 18, "%s", conf.local_mac);

    /* Destination MAC */
    netmap_argc += 2;
    netmap_argv = realloc(netmap_argv, (netmap_argc+1) * sizeof(char *));
    netmap_argv[netmap_argc-2] = "-D";
    netmap_argv[netmap_argc-1] = malloc(18 * sizeof(char));
    snprintf(netmap_argv[netmap_argc-1], 18, "%s", conf.remote_mac);

    /* Last arg must be NULL by convention */
    netmap_argv[netmap_argc] = NULL;

    /* Print a list of the arguments: */
    printf("Calling pkt-gen with arguments: ");
    for (int i = 0; i < netmap_argc; i++) {
        printf("%s ", netmap_argv[i]);
    }
    printf("\n");

    netmap_main_loop(netmap_argc, netmap_argv);
        
    return EXIT_SUCCESS;
}
