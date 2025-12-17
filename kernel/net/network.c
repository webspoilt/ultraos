/**
 * @file    kernel/net/network.c
 * @brief   Network stack implementation for UltraOS
 * @author  zeroday
 * @version 2.0.0
 * @date    2025-12-17
 * 
 * UltraOS - Advanced Multi-Architecture Operating System
 * Copyright (c) 2025 zeroday. All rights reserved.
 */

#include <ultraos_types.h>
#include <ultraos_config.h>
#include <kernel/core/kernel.h>
#include <kernel/net/network.h>
#include <kernel/memory/memory.h>

// Network interface list
static net_interface_t *net_interfaces[MAX_NET_INTERFACES];
static uint32_t net_interface_count = 0;

// Protocol handlers
static protocol_handler_t protocol_handlers[PROTOCOL_MAX];

// Network statistics
static net_stats_t net_stats = {0};

// Network buffer pool
static net_buf_t *net_buf_pool[NET_BUF_POOL_SIZE];
static uint32_t net_buf_free_count = NET_BUF_POOL_SIZE;

/**
 * @brief Initialize network subsystem
 * @return status_t Status of initialization
 */
status_t net_init(void)
{
    printk("Initializing network subsystem...\n");
    
    // Initialize network interfaces
    memset(net_interfaces, 0, sizeof(net_interfaces));
    net_interface_count = 0;
    
    // Initialize protocol handlers
    memset(protocol_handlers, 0, sizeof(protocol_handlers));
    
    // Initialize statistics
    memset(&net_stats, 0, sizeof(net_stats));
    
    // Initialize network buffer pool
    net_init_buffer_pool();
    
    // Register protocol handlers
    net_register_protocol_handlers();
    
    printk("Network subsystem initialized successfully\n");
    return STATUS_SUCCESS;
}

/**
 * @brief Register a network interface
 * @param iface Network interface
 * @return status_t Status of registration
 */
status_t net_register_interface(net_interface_t *iface)
{
    if (iface == NULL || iface->name == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (net_interface_count >= MAX_NET_INTERFACES) {
        return STATUS_OUT_OF_MEMORY;
    }
    
    // Check if interface already exists
    for (uint32_t i = 0; i < net_interface_count; i++) {
        if (strcmp(net_interfaces[i]->name, iface->name) == 0) {
            return STATUS_ALREADY_EXISTS;
        }
    }
    
    net_interfaces[net_interface_count] = iface;
    net_interface_count++;
    
    printk("Registered network interface: %s\n", iface->name);
    return STATUS_SUCCESS;
}

/**
 * @brief Unregister a network interface
 * @param name Interface name
 * @return status_t Status of unregistration
 */
status_t net_unregister_interface(const char *name)
{
    if (name == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Find interface
    uint32_t index = MAX_NET_INTERFACES;
    for (uint32_t i = 0; i < net_interface_count; i++) {
        if (strcmp(net_interfaces[i]->name, name) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == MAX_NET_INTERFACES) {
        return STATUS_NOT_FOUND;
    }
    
    // Call interface close function if available
    if (net_interfaces[index]->close) {
        net_interfaces[index]->close(net_interfaces[index]);
    }
    
    // Remove from list
    net_interface_count--;
    for (uint32_t i = index; i < net_interface_count; i++) {
        net_interfaces[i] = net_interfaces[i + 1];
    }
    
    printk("Unregistered network interface: %s\n", name);
    return STATUS_SUCCESS;
}

/**
 * @brief Get network interface by name
 * @param name Interface name
 * @return net_interface_t* Pointer to interface or NULL
 */
net_interface_t* net_get_interface(const char *name)
{
    if (name == NULL) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < net_interface_count; i++) {
        if (strcmp(net_interfaces[i]->name, name) == 0) {
            return net_interfaces[i];
        }
    }
    
    return NULL;
}

/**
 * @brief Send a network packet
 * @param iface Network interface
 * @param packet Network packet
 * @return status_t Status of operation
 */
status_t net_send_packet(net_interface_t *iface, net_packet_t *packet)
{
    if (iface == NULL || packet == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (!iface->is_up) {
        return STATUS_INVALID_STATE;
    }
    
    net_stats.total_sent++;
    net_stats.bytes_sent += packet->length;
    
    // Call interface send function
    if (iface->send) {
        return iface->send(iface, packet);
    }
    
    return STATUS_NOT_SUPPORTED;
}

/**
 * @brief Receive a network packet
 * @param iface Network interface
 * @param packet Network packet
 * @return status_t Status of operation
 */
status_t net_receive_packet(net_interface_t *iface, net_packet_t *packet)
{
    if (iface == NULL || packet == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    
    net_stats.total_received++;
    net_stats.bytes_received += packet->length;
    
    // Process packet based on protocol
    uint16_t protocol = packet->protocol;
    if (protocol < PROTOCOL_MAX && protocol_handlers[protocol] != NULL) {
        return protocol_handlers[protocol](iface, packet);
    }
    
    printk("Unknown protocol: 0x%04x\n", protocol);
    return STATUS_NOT_SUPPORTED;
}

/**
 * @brief Register a protocol handler
 * @param protocol Protocol number
 * @param handler Protocol handler function
 * @return status_t Status of registration
 */
status_t net_register_protocol(uint16_t protocol, protocol_handler_t handler)
{
    if (protocol >= PROTOCOL_MAX || handler == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (protocol_handlers[protocol] != NULL) {
        return STATUS_ALREADY_EXISTS;
    }
    
    protocol_handlers[protocol] = handler;
    return STATUS_SUCCESS;
}

/**
 * @brief Create a network packet
 * @param protocol Protocol number
 * @param length Packet length
 * @return net_packet_t* Pointer to packet or NULL
 */
net_packet_t* net_create_packet(uint16_t protocol, uint16_t length)
{
    if (length == 0 || length > NET_PACKET_MAX_SIZE) {
        return NULL;
    }
    
    net_packet_t *packet = (net_packet_t*)kmalloc(sizeof(net_packet_t) + length);
    if (packet == NULL) {
        return NULL;
    }
    
    memset(packet, 0, sizeof(net_packet_t));
    packet->protocol = protocol;
    packet->length = length;
    packet->data = (uint8_t*)(packet + 1);
    packet->ref_count = 1;
    
    return packet;
}

/**
 * @brief Free a network packet
 * @param packet Network packet
 * @return status_t Status of operation
 */
status_t net_free_packet(net_packet_t *packet)
{
    if (packet == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    
    packet->ref_count--;
    if (packet->ref_count == 0) {
        kfree(packet);
    }
    
    return STATUS_SUCCESS;
}

/**
 * @brief Initialize network buffer pool
 */
void net_init_buffer_pool(void)
{
    for (uint32_t i = 0; i < NET_BUF_POOL_SIZE; i++) {
        net_buf_t *buf = (net_buf_t*)kmalloc(sizeof(net_buf_t) + NET_BUF_SIZE);
        if (buf == NULL) {
            printk("WARNING: Failed to allocate network buffer %u\n", i);
            net_buf_pool[i] = NULL;
            continue;
        }
        
        memset(buf, 0, sizeof(net_buf_t));
        buf->data = (uint8_t*)(buf + 1);
        buf->size = NET_BUF_SIZE;
        buf->is_free = true;
        
        net_buf_pool[i] = buf;
    }
    
    net_buf_free_count = NET_BUF_POOL_SIZE;
}

/**
 * @brief Allocate a network buffer
 * @return net_buf_t* Pointer to buffer or NULL
 */
net_buf_t* net_alloc_buffer(void)
{
    for (uint32_t i = 0; i < NET_BUF_POOL_SIZE; i++) {
        if (net_buf_pool[i] != NULL && net_buf_pool[i]->is_free) {
            net_buf_pool[i]->is_free = false;
            net_buf_pool[i]->ref_count = 1;
            net_buf_free_count--;
            
            // Reset buffer
            memset(net_buf_pool[i]->data, 0, net_buf_pool[i]->size);
            net_buf_pool[i]->length = 0;
            net_buf_pool[i]->offset = 0;
            
            return net_buf_pool[i];
        }
    }
    
    return NULL;
}

/**
 * @brief Free a network buffer
 * @param buf Network buffer
 * @return status_t Status of operation
 */
status_t net_free_buffer(net_buf_t *buf)
{
    if (buf == NULL || !buf->is_free && buf->ref_count == 0) {
        return STATUS_INVALID_PARAMETER;
    }
    
    buf->ref_count--;
    if (buf->ref_count == 0) {
        buf->is_free = true;
        net_buf_free_count++;
        
        // Reset buffer
        memset(buf->data, 0, buf->size);
        buf->length = 0;
        buf->offset = 0;
    }
    
    return STATUS_SUCCESS;
}

/**
 * @brief Register protocol handlers
 */
void net_register_protocol_handlers(void)
{
    // Register ARP handler
    net_register_protocol(PROTOCOL_ARP, net_arp_handler);
    
    // Register IP handler
    net_register_protocol(PROTOCOL_IP, net_ip_handler);
    
    // Register TCP handler
    net_register_protocol(PROTOCOL_TCP, net_tcp_handler);
    
    // Register UDP handler
    net_register_protocol(PROTOCOL_UDP, net_udp_handler);
    
    // Register ICMP handler
    net_register_protocol(PROTOCOL_ICMP, net_icmp_handler);
}

/**
 * @brief ARP protocol handler
 * @param iface Network interface
 * @param packet Network packet
 * @return status_t Status of operation
 */
status_t net_arp_handler(net_interface_t *iface, net_packet_t *packet)
{
    // TODO: Implement ARP protocol handling
    printk("ARP packet received on %s\n", iface->name);
    return STATUS_SUCCESS;
}

/**
 * @brief IP protocol handler
 * @param iface Network interface
 * @param packet Network packet
 * @return status_t Status of operation
 */
status_t net_ip_handler(net_interface_t *iface, net_packet_t *packet)
{
    // TODO: Implement IP protocol handling
    printk("IP packet received on %s\n", iface->name);
    return STATUS_SUCCESS;
}

/**
 * @brief TCP protocol handler
 * @param iface Network interface
 * @param packet Network packet
 * @return status_t Status of operation
 */
status_t net_tcp_handler(net_interface_t *iface, net_packet_t *packet)
{
    // TODO: Implement TCP protocol handling
    printk("TCP packet received on %s\n", iface->name);
    return STATUS_SUCCESS;
}

/**
 * @brief UDP protocol handler
 * @param iface Network interface
 * @param packet Network packet
 * @return status_t Status of operation
 */
status_t net_udp_handler(net_interface_t *iface, net_packet_t *packet)
{
    // TODO: Implement UDP protocol handling
    printk("UDP packet received on %s\n", iface->name);
    return STATUS_SUCCESS;
}

/**
 * @brief ICMP protocol handler
 * @param iface Network interface
 * @param packet Network packet
 * @return status_t Status of operation
 */
status_t net_icmp_handler(net_interface_t *iface, net_packet_t *packet)
{
    // TODO: Implement ICMP protocol handling
    printk("ICMP packet received on %s\n", iface->name);
    return STATUS_SUCCESS;
}

/**
 * @brief Create a loopback network interface
 * @param name Interface name
 * @return net_interface_t* Pointer to interface or NULL
 */
net_interface_t* net_create_loopback(const char *name)
{
    net_interface_t *iface = (net_interface_t*)kmalloc(sizeof(net_interface_t));
    if (iface == NULL) {
        return NULL;
    }
    
    memset(iface, 0, sizeof(net_interface_t));
    iface->name = strdup(name);
    iface->type = NET_IFACE_TYPE_LOOPBACK;
    iface->is_up = true;
    iface->mtu = 65536; // Loopback has large MTU
    iface->flags = NET_IFACE_FLAG_UP;
    
    // Set loopback MAC address
    iface->hw_addr[0] = 0x00;
    iface->hw_addr[1] = 0x00;
    iface->hw_addr[2] = 0x00;
    iface->hw_addr[3] = 0x00;
    iface->hw_addr[4] = 0x00;
    iface->hw_addr[5] = 0x01;
    
    // Set loopback IP address
    iface->ip_addr = 0x7F000001; // 127.0.0.1
    
    return iface;
}

/**
 * @brief Get network statistics
 * @return net_stats_t* Pointer to statistics
 */
net_stats_t* net_get_stats(void)
{
    return &net_stats;
}

/**
 * @brief Reset network statistics
 * @return status_t Status of operation
 */
status_t net_reset_stats(void)
{
    memset(&net_stats, 0, sizeof(net_stats));
    return STATUS_SUCCESS;
}

/**
 * @brief List all network interfaces
 * @return status_t Status of operation
 */
status_t net_list_interfaces(void)
{
    printk("Network Interfaces:\n");
    printk("%-15s %-10s %-12s %-18s %-18s %-8s\n", 
           "Name", "Type", "State", "IP Address", "MAC Address", "MTU");
    printk("---------------------------------------------------------------------\n");
    
    for (uint32_t i = 0; i < net_interface_count; i++) {
        net_interface_t *iface = net_interfaces[i];
        
        // Format IP address
        uint8_t *ip_bytes = (uint8_t*)&iface->ip_addr;
        char ip_str[16];
        snprintf(ip_str, sizeof(ip_str), "%u.%u.%u.%u",
                ip_bytes[3], ip_bytes[2], ip_bytes[1], ip_bytes[0]);
        
        // Format MAC address
        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                iface->hw_addr[0], iface->hw_addr[1], iface->hw_addr[2],
                iface->hw_addr[3], iface->hw_addr[4], iface->hw_addr[5]);
        
        printk("%-15s %-10u %-12s %-18s %-18s %-8u\n",
               iface->name,
               iface->type,
               iface->is_up ? "UP" : "DOWN",
               ip_str,
               mac_str,
               iface->mtu);
    }
    
    return STATUS_SUCCESS;
}

/**
 * @brief Get network buffer pool statistics
 * @return status_t Status of operation
 */
status_t net_buffer_pool_stats(void)
{
    printk("Network Buffer Pool: %u/%u buffers available\n", 
           net_buf_free_count, NET_BUF_POOL_SIZE);
    return STATUS_SUCCESS;
}