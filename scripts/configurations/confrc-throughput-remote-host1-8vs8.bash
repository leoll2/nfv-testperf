#!/bin/bash

LXC_CONT_NAMES=(        \
    dpdk_c0             \
    dpdk_c1             \
    dpdk_c2             \
    dpdk_c3             \
    dpdk_c4             \
    dpdk_c5             \
    dpdk_c6             \
    dpdk_c7             \
)

LXC_CONT_VFS=(          \
    enp4s2              \
    enp4s2f1            \
    enp4s2f2            \
    enp4s2f3            \
    enp4s2f4            \
    enp4s2f5            \
    enp4s2f6            \
    enp4s2f7            \
)

LXC_CONT_MACS=(         \
    02:00:00:00:00:10   \
    02:00:00:00:00:11   \
    02:00:00:00:00:12   \
    02:00:00:00:00:13   \
    02:00:00:00:00:14   \
    02:00:00:00:00:15   \
    02:00:00:00:00:16   \
    02:00:00:00:00:17   \

)

LXC_CONT_IPS=(          \
    10.0.3.10           \
    10.0.3.11           \
    10.0.3.12           \
    10.0.3.13           \
    10.0.3.14           \
    10.0.3.15           \
    10.0.3.16           \
    10.0.3.17           \
)

LXC_CONT_NMASKS=(       \
    24                  \
    24                  \
    24                  \
    24                  \
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
    uio4                \
    uio5                \
    uio6                \
    uio7                \
)

LXC_CONT_VFPCIS=(       \
    04:02.0             \
    04:02.1             \
    04:02.2             \
    04:02.3             \
    04:02.4             \
    04:02.5             \
    04:02.6             \
    04:02.7             \
)

LXC_CONT_SOCKS=(        \
    sock0               \
    sock1               \
    sock2               \
    sock3               \
    sock4               \
    sock5               \
    sock6               \
    sock7               \
)

LXC_CONT_OTHER_IPS=(    \
    10.0.3.20           \
    10.0.3.21           \
    10.0.3.22           \
    10.0.3.23           \
    10.0.3.24           \
    10.0.3.25           \
    10.0.3.26           \
    10.0.3.27           \
)

LXC_CONT_OTHER_MACS=(   \
    02:00:00:00:00:20   \
    02:00:00:00:00:21   \
    02:00:00:00:00:22   \
    02:00:00:00:00:23   \
    02:00:00:00:00:24   \
    02:00:00:00:00:25   \
    02:00:00:00:00:26   \
    02:00:00:00:00:27   \
)

LXC_CONT_NETMAP_LOCAL_IF=( \
    netmap:iface_0_guest    \
    netmap:iface_1_guest    \
    netmap:iface_2_guest    \
    netmap:iface_3_guest    \
    netmap:iface_4_guest    \
    netmap:iface_5_guest    \
    netmap:iface_6_guest    \
    netmap:iface_7_guest    \
)

# Can be either send, recv, client, clientst, server
LXC_CONT_CMDNAMES=(     \
    send                \
    send                \
    send                \
    send                \
    send                \
    send                \
    send                \
    send                \
)
