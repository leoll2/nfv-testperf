#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "nfv_socket_simple.h"

#include <sys/socket.h>

#define UNUSED(x) ((void)x)

NFV_SOCKET_SIMPLE_SIGNATURE(void, init, nfv_conf_t conf)
{
    // TODO: must know the type of socket to be initialized
    struct nfv_socket_simple *sself = (struct nfv_socket_simple *)(self);

    // Initialize constant variables
    sself->sock_fd = conf->sock_fd;
    sself->is_raw = conf->is_raw;
    sself->use_mmsg = conf->use_mmsg;

    sself->used_size = (sself->is_raw) ? sself->super.packet_size : sself->super.payload_size;
    sself->base_offset = (sself->is_raw) ? OFFSET_PKT_PAYLOAD : 0;

    // Initialize addrlen sself->addrlen = sizeof(struct sockaddr_in);

    /* ------------------------- Memory allocation -------------------------- */

    // I need to allocate `burst_size` buffers of size `used_size`
    sself->packets = malloc(sizeof(buffer_t) * sself->super.burst_size);

    for (size_t i = 0; i < sself->super.burst_size; ++i)
    {
        sself->packets[i] = malloc(sizeof(byte_t) * sself->super.packet_size);
    }

    // These two are used for sendmmsg/recvmmsg API (both normal and raw
    // sockets)
    if (sself->use_mmsg)
    {
        // This must look like -> struct iovec iov[sself->super.burst_size][1];
        sself->iov = malloc(sizeof(struct iovec *));
        for (size_t i = 0; i < sself->super.burst_size; ++i)
        {
            sself->iov[i] = malloc(sizeof(struct iovec)); // Each has only one single element
        }

        sself->datagrams = malloc(sizeof(struct mmsghdr) * sself->super.burst_size);

        sself->src_addresses = malloc(sizeof(struct sockaddr_in) * sself->super.burst_size);
    }

    /* --------------------- Structures initialization ---------------------- */

    // Initialize the common header
    memset(sself->frame_hdr, 0, sizeof(sself->frame_hdr));

    // TODO: skilling a lot of memsets, check if this is cool or not FIXME: it
    // is NOT cool to skip if touch_data is set to false

    if (sself->use_mmsg)
    {
        // For sendmmsg/recvmmsg we need to build them for each message in the
        // burst

        // Building data structures for all the messages in the burst
        for (unsigned int i = 0; i < sself->super.burst_size; ++i)
        {
            sself->iov[i][0].iov_base = sself->packets[i];
            sself->iov[i][0].iov_len = sself->used_size;

            sself->datagrams[i].msg_hdr.msg_iov = sself->iov[i];
            sself->datagrams[i].msg_hdr.msg_iovlen = 1; // Only one vector element per message

            // Since for the server application addresses are NOT automatic (NOT
            // A CONNECTED SOCKET) we must specify where to put them
            sself->datagrams[i].msg_hdr.msg_name = &sself->src_addresses[i];

            // NOTICE: MUST REPEAT THIS OPERATION FOR EACH RECV LOOP
            sself->datagrams[i].msg_hdr.msg_namelen = sizeof(struct sockaddr_in);

            /* For UDP sockets, addresses are put automatically since they are
             * connected sockets. For RAW sockets we will build below a common
             * header to be used for all the packets, since this socket behaves
             * like a connected socket this is fine for send purposes FIXME:
             * check for receive purposes too
             */
        }
    }

    // TODO: probably frame_hdr riesco a tenerlo all'interno della init

    if (sself->is_raw)
    {
        /* For RAW sockets we need to build a frame header, the same for each
        * packet
        */
        dpdk_setup_pkt_headers(&sself->pkt_eth_hdr, &sself->pkt_ip_hdr, &sself->pkt_udp_hdr, conf);
        memcpy(sself->frame_hdr + OFFSET_PKT_ETHER, &sself->pkt_eth_hdr, sizeof(struct rte_ether_hdr));
        memcpy(sself->frame_hdr + OFFSET_PKT_IPV4, &sself->pkt_ip_hdr, sizeof(struct rte_ipv4_hdr));
        memcpy(sself->frame_hdr + OFFSET_PKT_UDP, &sself->pkt_udp_hdr, sizeof(struct rte_udp_hdr));

        /* Set the socket sending buffer size equal to the desired frame size It
         * doesn't seem to affect the system so much actually TODO: change this
         * if necessary: TODO: investigate if changes something, didn't see much
         * difference so far
         */
        // sock_set_sndbuff(conf->sock_fd, 128 * 1024 * 1024UL);

        // Fill the packet headers of all the buffers at initialization time, to
        // save some time later if possible

        // If using raw sockets, copy frame header into each packet
        for (size_t i = 0; i < sself->super.burst_size; ++i)
        {
            memcpy(sself->packets[i], sself->frame_hdr, PKT_HEADER_SIZE);
        }
    }

    /* ---------------------------------------------------------------------- */

    // Now it's time to initialize the pointers in the superclass NOTICE:  users
    // can directly request and manipulate payloads like this and the headers
    // will always be fine as they are
    for (size_t i = 0; i < sself->super.burst_size; ++i)
    {
        sself->super.payloads[i] = sself->packets[i] + sself->base_offset;
    }
}

// FIXME: if I give less than burst_size here, I should remember it because
// otherwise the send would send too many packets
NFV_SOCKET_SIMPLE_SIGNATURE(void, request_out_buffers, buffer_t *buffers, size_t size, size_t howmany)
{
    struct nfv_socket_simple *sself = (struct nfv_socket_simple *)(self);

    // TODO: ASSUMES howmany <= burst_size
    for (size_t i = 0; i < howmany; ++i)
    {
        buffers[i] = sself->super.payloads[i];
    }
}

NFV_SOCKET_SIMPLE_SIGNATURE(ssize_t, send)
{
    struct nfv_socket_simple *sself = (struct nfv_socket_simple *)(self);

    int num_sent = 0;

    if (sself->use_mmsg)
    {
        // Use one only system call
        num_sent = sendmmsg(sself->sock_fd, sself->datagrams, sself->super.burst_size, 0);
    }
    else
    {
        for (size_t i = 0; i < sself->super.burst_size; ++i)
        {
            // For RAW sockets, the header is already in its place from
            // initialization. If you want to copy it again, uncomment the
            // following line:

            // memcpy(sself->packets[i], sself->frame_hdr, PKT_HEADER_SIZE);

            // TODO: transition to sendmdg
            int res = send(sself->sock_fd, sself->packets[i], sself->used_size, 0);
            if (res == sself->used_size)
            {
                ++num_sent;
            }
        }
    }

    return num_sent;
}

// BUG: for the server there is no way to check who sent a message and construct
// the appropriate headers as of now. ALSO BUG: for raw sockets should check
// that the destination address is this address
NFV_SOCKET_SIMPLE_SIGNATURE(ssize_t, recv, buffer_t *buffers, size_t size, size_t howmany)
{
    struct nfv_socket_simple *sself = (struct nfv_socket_simple *)(self);

    int num_recv = 0;

    if (sself->use_mmsg)
    {
        for (size_t i = 0; i < sself->super.burst_size; ++i)
        {
            sself->datagrams[i].msg_hdr.msg_namelen = sizeof(struct sockaddr_in);
        }

        // Use one only system call, fills header data too
        num_recv = recvmmsg(sself->sock_fd, sself->datagrams, sself->super.burst_size, 0, NULL);
    }
    else
    {
        for (size_t i = 0; i < sself->super.burst_size; ++i)
        {
            // NOTICE: If some failures occur, num_recv points to the next
            // packet ready to be filled

            // FIXME: why was this payload_len?                              |
            //                                                              \ /
            //                                                               °
            int res = recv(sself->sock_fd, sself->packets[num_recv], sself->used_size, 0);

            // In UDP/RAW you either receive it all or not receive anything at
            // all
            if (res == sself->used_size)
            {
                ++num_recv;
            }
        }
    }

    return num_recv;
}

// CHECK: should this be simply equal to send with some arrangements of the
// headers? CHECK: I think for sendmmsg it could be as easy as calling simply
// sendmsg/sendmmsg, because the headers are updated by recvmsg/recvmmsg calls
NFV_SOCKET_SIMPLE_SIGNATURE(ssize_t, send_back)
{
    // TODO:
    UNUSED(self);
    return 0;
}

NFV_SOCKET_SIMPLE_SIGNATURE(void, free_buffers)
{
    // TODO: For now this could ne a NOP for simple sockets
    UNUSED(self);
}