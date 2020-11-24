#ifndef NFV_SOCKET_NETMAP_H
#define NFV_SOCKET_NETMAP_H

#include "hdr_tools.h"
#include "nfv_socket.h"

#include <libnetmap.h>

/* ---------------------------- TYPE DEFINITIONS ---------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------- MACROS (FOR FUNCTION PROTOTYPES) -------------------- */

#define NFV_NETMAP_SIGNATURE(return_t, name, ...)                              \
    NFV_SIGNATURE(return_t, netmap_##name, ##__VA_ARGS__)

/* ----------------------- CLASS FUNCTION PROTOTYPES ------------------------ */

extern NFV_NETMAP_SIGNATURE(void, init, config_ptr conf);

extern NFV_NETMAP_SIGNATURE(size_t, request_out_buffers, buffer_t buffers[],
                            size_t howmany);

extern NFV_NETMAP_SIGNATURE(ssize_t, send, size_t howmany);

extern NFV_NETMAP_SIGNATURE(ssize_t, recv, buffer_t buffers[], size_t howmany);

extern NFV_NETMAP_SIGNATURE(ssize_t, send_back, size_t howmany);

/* ---------------------------- CLASS DEFINITION ---------------------------- */

struct nfv_socket_netmap {
    /* Base class holder */
    struct nfv_socket super;

    /* Header structures, typically the same for each message */
    struct pkt_hdr outgoing_hdr;
    struct pkt_hdr incoming_hdr;

    // TODO add the required structures
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* NFV_SOCKET_NETMAP_H */
