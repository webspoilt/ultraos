/**
 * @file    include/ultraos_devices.h
 * @brief   Device management header definitions for UltraOS
 * @author  zeroday
 * @version 2.0.0
 * @date    2025-12-17
 * 
 * UltraOS - Advanced Multi-Architecture Operating System
 * Copyright (c) 2025 zeroday. All rights reserved.
 */

#ifndef ULTRAOS_DEVICES_H
#define ULTRAOS_DEVICES_H

#include <ultraos_types.h>

// Device constants
#define MAX_DEVICES           256
#define MAX_DEVICE_NAME       64
#define MAX_FILE_DESCRIPTORS  1024

// Device types
typedef enum {
    DEVICE_TYPE_NONE = 0,
    DEVICE_TYPE_CHAR,          // Character device
    DEVICE_TYPE_BLOCK,         // Block device
    DEVICE_TYPE_NETWORK,       // Network device
    DEVICE_TYPE_BUS,           // Bus device
    DEVICE_TYPE_INPUT,         // Input device
    DEVICE_TYPE_DISPLAY,       // Display device
    DEVICE_TYPE_AUDIO,         // Audio device
    DEVICE_TYPE_STORAGE,       // Storage device
    DEVICE_TYPE_SENSOR,        // Sensor device
    DEVICE_TYPE_ACTUATOR,      // Actuator device
    DEVICE_TYPE_MAX
} device_type_t;

// Device flags
#define DEVICE_FLAG_PRESENT      0x00000001
#define DEVICE_FLAG_ENABLED      0x00000002
#define DEVICE_FLAG_REMOVABLE    0x00000004
#define DEVICE_FLAG_HOTPLUG      0x00000008
#define DEVICE_FLAG_POWER_MANAGED 0x00000010

// Device I/O control commands
#define DIOCGETINFO      0x0001
#define DIOCSETFLAGS     0x0002
#define DIOCGETFLAGS     0x0003
#define DIOCGETSIZE      0x0004
#define DIOCRESET        0x0005
#define DIOCSUSPEND      0x0006
#define DIOCRESUME       0x0007
#define DIOCGETSTATE     0x0008

// Device driver structure
typedef struct device_driver {
    device_type_t type;        // Device type
    char name[MAX_DEVICE_NAME]; // Driver name
    
    // Driver function pointers
    status_t (*probe)(void *device);
    status_t (*remove)(void *device);
    status_t (*open)(void *device, uint32_t flags);
    status_t (*close)(void *device);
    int64_t (*read)(void *device, void *buf, uint64_t count);
    int64_t (*write)(void *device, const void *buf, uint64_t count);
    int64_t (*ioctl)(void *device, uint32_t cmd, uint64_t arg);
    status_t (*suspend)(void *device);
    status_t (*resume)(void *device);
} device_driver_t;

// Device structure
typedef struct device {
    device_type_t type;        // Device type
    uint32_t id;               // Device ID
    char name[MAX_DEVICE_NAME]; // Device name
    uint32_t flags;            // Device flags
    bool is_used;              // Is device slot used
    void *data;                // Private device data
    device_driver_t *driver;   // Device driver
    uint32_t ref_count;        // Reference count
    
    // Device attributes
    uint32_t minor;            // Minor device number
    uint32_t major;            // Major device number
    
    // Performance metrics
    uint64_t read_count;       // Number of read operations
    uint64_t write_count;      // Number of write operations
    uint64_t ioctl_count;      // Number of ioctl operations
    uint64_t read_bytes;       // Bytes read
    uint64_t write_bytes;      // Bytes written
} device_t;

// File descriptor structure
typedef struct file_descriptor {
    bool is_open;              // Is file descriptor open
    uint32_t flags;            // File flags
    uint32_t mode;             // File mode
    device_t *device;          // Associated device
    uint64_t position;         // Current position
    uint32_t inode;            // Inode number
    uint32_t ref_count;        // Reference count
} file_descriptor_t;

// Device statistics structure
typedef struct device_stats {
    uint32_t total_devices;    // Total number of devices
    uint32_t total_opens;      // Total device opens
    uint32_t total_closes;     // Total device closes
    uint32_t total_reads;      // Total read operations
    uint32_t total_writes;     // Total write operations
    uint32_t total_ioctls;     // Total ioctl operations
    uint64_t total_read_bytes; // Total bytes read
    uint64_t total_write_bytes; // Total bytes written
    uint32_t errors;           // Total errors
} device_stats_t;

// Function pointer types
typedef status_t (*device_probe_t)(void *device);
typedef status_t (*device_remove_t)(void *device);
typedef status_t (*device_open_t)(device_t *device, uint32_t flags);
typedef status_t (*device_close_t)(device_t *device);
typedef int64_t (*device_read_t)(device_t *device, void *buf, uint64_t count);
typedef int64_t (*device_write_t)(device_t *device, const void *buf, uint64_t count);
typedef int64_t (*device_ioctl_t)(device_t *device, uint32_t cmd, uint64_t arg);
typedef status_t (*device_suspend_t)(device_t *device);
typedef status_t (*device_resume_t)(device_t *device);

// Function prototypes

// Initialization
status_t devices_init(void);

// Driver management
status_t devices_register_driver(device_driver_t *driver);
status_t devices_unregister_driver(device_type_t type);

// Device management
status_t devices_create_device(device_type_t type, const char *name, 
                               device_driver_t *driver, void *data);
status_t devices_remove_device(device_t *device);
device_t* devices_find_by_name(const char *name);
device_t* devices_find_by_id(uint32_t id);

// Device operations
device_t* devices_open(const char *name, uint32_t flags);
status_t devices_close(device_t *device);
int64_t devices_read(device_t *device, void *buf, uint64_t count);
int64_t devices_write(device_t *device, const void *buf, uint64_t count);
int64_t devices_ioctl(device_t *device, uint32_t cmd, uint64_t arg);

// Device type specific creation functions
status_t devices_create_chardev(const char *name, device_driver_t *driver, void *data);
status_t devices_create_blockdev(const char *name, device_driver_t *driver, void *data);
status_t devices_create_netdev(const char *name, device_driver_t *driver, void *data);
status_t devices_create_busdev(const char *name, device_driver_t *driver, void *data);

// Driver helper functions
device_driver_t* devices_create_driver(device_type_t type, const char *name,
                                       device_open_t open, device_close_t close,
                                       device_read_t read, device_write_t write,
                                       device_ioctl_t ioctl);

// Statistics and monitoring
device_stats_t* devices_get_stats(void);
status_t devices_reset_stats(void);
status_t devices_list(void);

// Built-in driver registration
status_t devices_register_builtin_drivers(void);

// Utility macros
#define DEVICE_MAKE_ID(type, minor)  (((type) << 16) | (minor))
#define DEVICE_GET_TYPE(id)          (((id) >> 16) & 0xFFFF)
#define DEVICE_GET_MINOR(id)         ((id) & 0xFFFF)

// Device type checks
#define DEVICE_IS_CHAR(type)         ((type) == DEVICE_TYPE_CHAR)
#define DEVICE_IS_BLOCK(type)        ((type) == DEVICE_TYPE_BLOCK)
#define DEVICE_IS_NETWORK(type)      ((type) == DEVICE_TYPE_NETWORK)
#define DEVICE_IS_INPUT(type)        ((type) == DEVICE_TYPE_INPUT)
#define DEVICE_IS_STORAGE(type)      ((type) == DEVICE_TYPE_STORAGE)

// Major device numbers (example)
#define MAJOR_CHAR_DEVICES           8
#define MAJOR_BLOCK_DEVICES          8
#define MAJOR_NETWORK_DEVICES        8

// Common minor device numbers
#define MINOR_CONSOLE               0
#define MINOR_TTY0                  0
#define MINOR_TTY1                  1
#define MINOR_TTY2                  2
#define MINOR_TTY3                  3
#define MINOR_SERIAL0               64
#define MINOR_SERIAL1               65
#define MINOR_SERIAL2               66
#define MINOR_SERIAL3               67
#define MINOR_HDA0                  0
#define MINOR_HDA1                  1
#define MINOR_HDA2                  2
#define MINOR_HDA3                  3
#define MINOR_ETH0                  0
#define MINOR_ETH1                  1
#define MINOR_ETH2                  2
#define MINOR_ETH3                  3

#endif // ULTRAOS_DEVICES_H