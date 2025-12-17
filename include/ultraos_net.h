/**
 * @file    include/ultraos_net.h
 * @brief   Network subsystem header definitions for UltraOS
 * @author  zeroday
 * @version 2.0.0
 * @date    2025-12-17
 * 
 * UltraOS - Advanced Multi-Architecture Operating System
 * Copyright (c) 2025 zeroday. All rights reserved.
 */

#ifndef ULTRAOS_NET_H
#define ULTRAOS_NET_H

#include <ultraos_types.h>

// Network constants
#define MAX_NET_INTERFACES      32
#define MAX_NET_NAME_LENGTH     32
#define MAX_PACKET_SIZE         65536
#define NET_BUF_POOL_SIZE       256
#define NET_BUF_SIZE            2048
#define NET_PACKET_MAX_SIZE     65535

// Protocol numbers
#define PROTOCOL_HOPOPT         0
#define PROTOCOL_ICMP           1
#define PROTOCOL_IGMP           2
#define PROTOCOL_GGP            3
#define PROTOCOL_IPV4           4
#define PROTOCOL_TCP            6
#define PROTOCOL_UDP            17
#define PROTOCOL_UDPLITE        136
#define PROTOCOL_SCTP           132
#define PROTOCOL_RAW            255
#define PROTOCOL_MAX            256

// ARP protocol number
#define PROTOCOL_ARP            0x0806
#define PROTOCOL_IP             0x0800

// Interface types
typedef enum {
    NET_IFACE_TYPE_ETHERNET = 0,
    NET_IFACE_TYPE_LOOPBACK = 1,
    NET_IFACE_TYPE_WIFI     = 2,
    NET_IFACE_TYPE_TOKEN_RING = 3,
    NET_IFACE_TYPE_FDDI     = 4,
    NET_IFACE_TYPE_MAX
} net_iface_type_t;

// Interface flags
#define NET_IFACE_FLAG_UP       0x00000001
#define NET_IFACE_FLAG_BROADCAST 0x00000002
#define NET_IFACE_FLAG_PROMISC  0x00000004
#define NET_IFACE_FLAG_MULTICAST 0x00000008
#define NET_IFACE_FLAG_NOARP    0x00000010
#define NET_IFACE_FLAG_POINTOPOINT 0x00000020
#define NET_IFACE_FLAG_ALLMULTI 0x00000040

// Network packet structure
typedef struct net_packet {
    uint16_t protocol;      // Protocol number
    uint16_t length;        // Packet length
    uint8_t *data;          // Packet data
    uint8_t src_mac[6];     // Source MAC address
    uint8_t dst_mac[6];     // Destination MAC address
    uint32_t src_ip;        // Source IP address
    uint32_t dst_ip;        // Destination IP address
    uint16_t src_port;      // Source port
    uint16_t dst_port;      // Destination port
    uint32_t ref_count;     // Reference count
} net_packet_t;

// Network buffer structure
typedef struct net_buf {
    uint8_t *data;          // Buffer data
    uint32_t size;          // Buffer size
    uint32_t length;        // Current length
    uint32_t offset;        // Current offset
    bool is_free;           // Is buffer free
    uint32_t ref_count;     // Reference count
} net_buf_t;

// Network interface structure
typedef struct net_interface {
    char name[MAX_NET_NAME_LENGTH];  // Interface name
    net_iface_type_t type;           // Interface type
    uint32_t flags;                  // Interface flags
    bool is_up;                      // Is interface up
    uint32_t mtu;                    // Maximum transmission unit
    uint8_t hw_addr[6];              // Hardware (MAC) address
    uint32_t ip_addr;                // IP address
    uint32_t netmask;                // Network mask
    uint32_t gateway;                // Gateway address
    uint32_t broadcast;              // Broadcast address
    void *priv_data;                 // Private driver data
    
    // Driver function pointers
    status_t (*open)(struct net_interface *iface);
    status_t (*close)(struct net_interface *iface);
    status_t (*send)(struct net_interface *iface, net_packet_t *packet);
    status_t (*recv)(struct net_interface *iface, net_packet_t *packet);
    status_t (*ioctl)(struct net_interface *iface, uint32_t cmd, void *arg);
} net_interface_t;

// Protocol handler function type
typedef status_t (*protocol_handler_t)(net_interface_t *iface, net_packet_t *packet);

// Network statistics structure
typedef struct net_stats {
    uint64_t total_sent;     // Total packets sent
    uint64_t total_received; // Total packets received
    uint64_t bytes_sent;     // Total bytes sent
    uint64_t bytes_received; // Total bytes received
    uint32_t errors_sent;    // Send errors
    uint32_t errors_received; // Receive errors
    uint32_t dropped;        // Dropped packets
} net_stats_t;

// Function prototypes

// Initialization
status_t net_init(void);

// Interface management
status_t net_register_interface(net_interface_t *iface);
status_t net_unregister_interface(const char *name);
net_interface_t* net_get_interface(const char *name);

// Packet handling
status_t net_send_packet(net_interface_t *iface, net_packet_t *packet);
status_t net_receive_packet(net_interface_t *iface, net_packet_t *packet);

// Protocol handling
status_t net_register_protocol(uint16_t protocol, protocol_handler_t handler);

// Packet allocation/deallocation
net_packet_t* net_create_packet(uint16_t protocol, uint16_t length);
status_t net_free_packet(net_packet_t *packet);

// Buffer management
net_buf_t* net_alloc_buffer(void);
status_t net_free_buffer(net_buf_t *buf);

// Statistics
net_stats_t* net_get_stats(void);
status_t net_reset_stats(void);

// Utility functions
status_t net_list_interfaces(void);
status_t net_buffer_pool_stats(void);
net_interface_t* net_create_loopback(const char *name);

// Protocol handlers (internal)
status_t net_arp_handler(net_interface_t *iface, net_packet_t *packet);
status_t net_ip_handler(net_interface_t *iface, net_packet_t *packet);
status_t net_tcp_handler(net_interface_t *iface, net_packet_t *packet);
status_t net_udp_handler(net_interface_t *iface, net_packet_t *packet);
status_t net_icmp_handler(net_interface_t *iface, net_packet_t *packet);

// Utility functions
static inline uint32_t net_ip_to_uint32(const uint8_t *ip)
{
    return ((uint32_t)ip[0] << 24) | ((uint32_t)ip[1] << 16) | 
           ((uint32_t)ip[2] << 8) | ((uint32_t)ip[3]);
}

static inline void net_uint32_to_ip(uint32_t ip, uint8_t *result)
{
    result[0] = (ip >> 24) & 0xFF;
    result[1] = (ip >> 16) & 0xFF;
    result[2] = (ip >> 8) & 0xFF;
    result[3] = ip & 0xFF;
}

static inline bool net_is_multicast_ip(uint32_t ip)
{
    return (ip & 0xF0000000) == 0xE0000000; // 224.0.0.0/4
}

static inline bool net_is_broadcast_ip(uint32_t ip, uint32_t netmask)
{
    return (ip & ~netmask) == (~netmask);
}

static inline bool net_is_loopback_ip(uint32_t ip)
{
    return (ip & 0xFF000000) == 0x7F000000; // 127.0.0.0/8
}

#endif // ULTRAOS_NET_H