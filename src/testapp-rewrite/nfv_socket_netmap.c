#include "config.h"
#include "constants.h"
#include "nfv_socket_netmap.h"

#define netmap_packet_start(p, t)       <TODO>
#define netmap_packet_offset(p, t, o)   <TODO>

// static inline byte_t *netmap_payload(...) {}     // TODO

static inline NFV_NETMAP_SIGNATURE(void, free_buffers) {
    struct nfv_socket_netmap *sself = (struct nfv_socket_netmap *)(self);
    // TODO
}

static inline NFV_NETMAP_SIGNATURE(void, fill_buffer_array, buffer_t buffers[],
                                 size_t howmany) {
    struct nfv_socket_netmap *sself = (struct nfv_socket_netmap *)(self);
 
    // TODO
}

// static inline void netmap_prepare_packet(...) {}  // TODO

NFV_NETMAP_SIGNATURE(void, init, config_ptr conf) {
    struct nfv_socket_netmap *sself = (struct nfv_socket_netmap *)(self);

    // TODO
}

NFV_NETMAP_SIGNATURE(size_t, request_out_buffers, buffer_t buffers[],
                   size_t howmany) {
    struct nfv_socket_netmap *sself = (struct nfv_socket_netmap *)(self);

    // TODO
}

NFV_NETMAP_SIGNATURE(ssize_t, send, size_t howmany) {
    struct nfv_socket_netmap *sself = (struct nfv_socket_netmap *)(self);
    size_t num_sent;

    // TODO

    return num_sent;
}

NFV_NETMAP_SIGNATURE(ssize_t, recv, buffer_t buffers[], size_t howmany) {
    struct nfv_socket_netmap *sself = (struct nfv_socket_netmap *)(self);
    size_t num_recv;
    size_t num_recv_good;
    size_t i;

    // TODO

    return num_recv_good;
}

NFV_NETMAP_SIGNATURE(ssize_t, send_back, size_t howmany) {
    struct nfv_socket_netmap *sself = (struct nfv_socket_netmap *)(self);

    // TODO

    return nfv_socket_netmap_send(self, howmany);
}
