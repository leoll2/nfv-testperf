#!/bin/bash

LXC_CONT_NAMES=(        \
    dpdk_c0             \
    dpdk_c1             \
    dpdk_c2             \
    dpdk_c3             \
)

LXC_CONT_VFS=(          \
    enp4s2              \
    enp4s2f1            \
    enp4s2f2            \
    enp4s2f3            \
)

LXC_CONT_MACS=(         \
    02:00:00:00:00:10   \
    02:00:00:00:00:11   \
    02:00:00:00:00:12   \
    02:00:00:00:00:13   \
)

LXC_CONT_IPS=(          \
    10.0.3.10           \
    10.0.3.11           \
    10.0.3.12           \
    10.0.3.13           \
)

LXC_CONT_NMASKS=(       \
    24                  \
    24                  \
    24                  \
    24                  \
)

LXC_CONT_VFDEVS=(       \
    uio0                \
    uio1                \
    uio2                \
    uio3                \
)

LXC_CONT_VFPCIS=(       \
    04:02.0             \
    04:02.1             \
    04:02.2             \
    04:02.3             \
)

LXC_CONT_SOCKS=(        \
    sock0               \
    sock1               \
    sock2               \
    sock3               \
)

LXC_CONT_OTHER_IPS=(    \
    10.0.3.11           \
    10.0.3.10           \
    10.0.3.13           \
    10.0.3.12           \
)

LXC_CONT_OTHER_MACS=(   \
    02:00:00:00:00:11   \
    02:00:00:00:00:10   \
    02:00:00:00:00:13   \
    02:00:00:00:00:12   \
)

LXC_CONT_NETMAP_LOCAL_IF=( \
    netmap:iface_0_guest    \
    netmap:iface_1_guest    \
    netmap:iface_2_guest    \
    netmap:iface_3_guest    \
)

# Can be either send, recv, client, clientst, server
LXC_CONT_CMDNAMES=(     \
    send                \
    recv                \
    send                \
    recv                \
)
