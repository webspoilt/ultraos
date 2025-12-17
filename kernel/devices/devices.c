/**
 * @file    kernel/devices/devices.c
 * @brief   Device management subsystem for UltraOS
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
#include <kernel/devices/devices.h>

// Device driver registry
static device_driver_t *device_drivers[DEVICE_TYPE_MAX] = {NULL};

// Device table
static device_t devices[MAX_DEVICES];
static uint32_t device_count = 0;

// Device statistics
static device_stats_t device_stats = {0};

/**
 * @brief Initialize device management subsystem
 * @return status_t Status of initialization
 */
status_t devices_init(void)
{
    printk("Initializing device management subsystem...\n");
    
    // Initialize device table
    memset(devices, 0, sizeof(devices));
    device_count = 0;
    
    // Initialize driver registry
    memset(device_drivers, 0, sizeof(device_drivers));
    
    // Initialize statistics
    memset(&device_stats, 0, sizeof(device_stats));
    
    // Register built-in drivers
    status_t ret = devices_register_builtin_drivers();
    if (ret != STATUS_SUCCESS) {
        printk("Warning: Failed to register some built-in drivers\n");
    }
    
    printk("Device management initialized: %u devices registered\n", device_count);
    return STATUS_SUCCESS;
}

/**
 * @brief Register a device driver
 * @param driver Pointer to device driver
 * @return status_t Status of registration
 */
status_t devices_register_driver(device_driver_t *driver)
{
    if (driver == NULL || driver->type >= DEVICE_TYPE_MAX) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (device_drivers[driver->type] != NULL) {
        return STATUS_ALREADY_EXISTS;
    }
    
    device_drivers[driver->type] = driver;
    
    printk("Registered device driver: %s for type %u\n", 
           driver->name, driver->type);
    
    return STATUS_SUCCESS;
}

/**
 * @brief Create and register a new device
 * @param type Device type
 * @param name Device name
 * @param driver Driver for this device
 * @param data Private device data
 * @return status_t Status of creation
 */
status_t devices_create_device(device_type_t type, const char *name, 
                               device_driver_t *driver, void *data)
{
    if (driver == NULL || name == NULL || type >= DEVICE_TYPE_MAX) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (device_count >= MAX_DEVICES) {
        return STATUS_OUT_OF_MEMORY;
    }
    
    // Find free device slot
    uint32_t index = 0;
    for (uint32_t i = 0; i < MAX_DEVICES; i++) {
        if (!devices[i].is_used) {
            index = i;
            break;
        }
    }
    
    // Initialize device structure
    device_t *dev = &devices[index];
    memset(dev, 0, sizeof(device_t));
    
    dev->type = type;
    dev->driver = driver;
    dev->data = data;
    dev->is_used = true;
    dev->ref_count = 1;
    
    // Copy device name
    strncpy(dev->name, name, MAX_DEVICE_NAME - 1);
    dev->name[MAX_DEVICE_NAME - 1] = '\0';
    
    // Generate device ID
    dev->id = (type << 16) | index;
    
    device_count++;
    
    printk("Created device: %s (ID: 0x%08x, type: %u)\n", 
           dev->name, dev->id, dev->type);
    
    return STATUS_SUCCESS;
}

/**
 * @brief Find a device by name
 * @param name Device name
 * @return device_t* Pointer to device or NULL if not found
 */
device_t* devices_find_by_name(const char *name)
{
    if (name == NULL) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].is_used && 
            strcmp(devices[i].name, name) == 0) {
            return &devices[i];
        }
    }
    
    return NULL;
}

/**
 * @brief Find a device by ID
 * @param id Device ID
 * @return device_t* Pointer to device or NULL if not found
 */
device_t* devices_find_by_id(uint32_t id)
{
    uint16_t type = (id >> 16) & 0xFFFF;
    uint16_t index = id & 0xFFFF;
    
    if (type >= DEVICE_TYPE_MAX || index >= MAX_DEVICES) {
        return NULL;
    }
    
    device_t *dev = &devices[index];
    if (dev->is_used && dev->id == id) {
        return dev;
    }
    
    return NULL;
}

/**
 * @brief Open a device
 * @param name Device name
 * @param flags Open flags
 * @return device_t* Pointer to opened device or NULL
 */
device_t* devices_open(const char *name, uint32_t flags)
{
    device_t *dev = devices_find_by_name(name);
    if (dev == NULL) {
        return NULL;
    }
    
    // Call driver open function if available
    if (dev->driver && dev->driver->open) {
        int32_t ret = dev->driver->open(dev, flags);
        if (ret < 0) {
            return NULL;
        }
    }
    
    dev->ref_count++;
    device_stats.total_opens++;
    
    return dev;
}

/**
 * @brief Close a device
 * @param dev Device pointer
 * @return status_t Status of operation
 */
status_t devices_close(device_t *dev)
{
    if (dev == NULL || !dev->is_used) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (dev->ref_count == 0) {
        return STATUS_INVALID_STATE;
    }
    
    // Call driver close function if available
    if (dev->driver && dev->driver->close) {
        dev->driver->close(dev);
    }
    
    dev->ref_count--;
    device_stats.total_closes++;
    
    if (dev->ref_count == 0) {
        // Device is now completely closed
        device_stats.total_closes++;
    }
    
    return STATUS_SUCCESS;
}

/**
 * @brief Read from a device
 * @param dev Device pointer
 * @param buf Buffer to read into
 * @param count Number of bytes to read
 * @return int64_t Number of bytes read or error code
 */
int64_t devices_read(device_t *dev, void *buf, uint64_t count)
{
    if (dev == NULL || !dev->is_used || buf == NULL || count == 0) {
        return -EINVAL;
    }
    
    device_stats.total_reads++;
    
    if (dev->driver && dev->driver->read) {
        return dev->driver->read(dev, buf, count);
    }
    
    return -ENOTSUP;
}

/**
 * @brief Write to a device
 * @param dev Device pointer
 * @param buf Buffer to write from
 * @param count Number of bytes to write
 * @return int64_t Number of bytes written or error code
 */
int64_t devices_write(device_t *dev, const void *buf, uint64_t count)
{
    if (dev == NULL || !dev->is_used || buf == NULL || count == 0) {
        return -EINVAL;
    }
    
    device_stats.total_writes++;
    
    if (dev->driver && dev->driver->write) {
        return dev->driver->write(dev, buf, count);
    }
    
    return -ENOTSUP;
}

/**
 * @brief Perform device control operation
 * @param dev Device pointer
 * @param cmd Control command
 * @param arg Command argument
 * @return int64_t Result of operation or error code
 */
int64_t devices_ioctl(device_t *dev, uint32_t cmd, uint64_t arg)
{
    if (dev == NULL || !dev->is_used) {
        return -EINVAL;
    }
    
    device_stats.total_ioctls++;
    
    if (dev->driver && dev->driver->ioctl) {
        return dev->driver->ioctl(dev, cmd, arg);
    }
    
    return -ENOTSUP;
}

/**
 * @brief Register built-in device drivers
 * @return status_t Status of registration
 */
status_t devices_register_builtin_drivers(void)
{
    // TODO: Implement built-in device drivers
    // - Serial port driver (COM1, COM2, etc.)
    // - VGA text mode driver
    // - PS/2 keyboard and mouse drivers
    // - PIT timer driver
    // - RTC driver
    
    printk("Built-in device drivers registration not yet implemented\n");
    return STATUS_SUCCESS;
}

/**
 * @brief Get device statistics
 * @return device_stats_t* Pointer to statistics
 */
device_stats_t* devices_get_stats(void)
{
    return &device_stats;
}

/**
 * @brief Reset device statistics
 * @return status_t Status of operation
 */
status_t devices_reset_stats(void)
{
    memset(&device_stats, 0, sizeof(device_stats));
    return STATUS_SUCCESS;
}

/**
 * @brief List all registered devices
 * @return status_t Status of operation
 */
status_t devices_list(void)
{
    printk("Device List:\n");
    printk("%-20s %-10s %-10s %-10s\n", "Name", "Type", "ID", "RefCount");
    printk("-----------------------------------------------------------\n");
    
    for (uint32_t i = 0; i < MAX_DEVICES; i++) {
        if (devices[i].is_used) {
            printk("%-20s %-10u 0x%-8x %-10u\n",
                   devices[i].name,
                   devices[i].type,
                   devices[i].id,
                   devices[i].ref_count);
        }
    }
    
    printk("Total devices: %u\n", device_count);
    return STATUS_SUCCESS;
}

/**
 * @brief Create a character device
 * @param name Device name
 * @param driver Driver for this device
 * @param data Private device data
 * @return status_t Status of creation
 */
status_t devices_create_chardev(const char *name, device_driver_t *driver, void *data)
{
    return devices_create_device(DEVICE_TYPE_CHAR, name, driver, data);
}

/**
 * @brief Create a block device
 * @param name Device name
 * @param driver Driver for this device
 * @param data Private device data
 * @return status_t Status of creation
 */
status_t devices_create_blockdev(const char *name, device_driver_t *driver, void *data)
{
    return devices_create_device(DEVICE_TYPE_BLOCK, name, driver, data);
}

/**
 * @brief Create a network device
 * @param name Device name
 * @param driver Driver for this device
 * @param data Private device data
 * @return status_t Status of creation
 */
status_t devices_create_netdev(const char *name, device_driver_t *driver, void *data)
{
    return devices_create_device(DEVICE_TYPE_NETWORK, name, driver, data);
}

/**
 * @brief Create a bus device
 * @param name Device name
 * @param driver Driver for this device
 * @param data Private device data
 * @return status_t Status of creation
 */
status_t devices_create_busdev(const char *name, device_driver_t *driver, void *data)
{
    return devices_create_device(DEVICE_TYPE_BUS, name, driver, data);
}

/**
 * @brief Device driver registration helper function
 * @param type Device type
 * @param name Driver name
 * @param open Open function
 * @param close Close function
 * @param read Read function
 * @param write Write function
 * @param ioctl IO control function
 * @return device_driver_t* Pointer to created driver
 */
device_driver_t* devices_create_driver(device_type_t type, const char *name,
                                       device_open_t open, device_close_t close,
                                       device_read_t read, device_write_t write,
                                       device_ioctl_t ioctl)
{
    device_driver_t *driver = (device_driver_t*)kmalloc(sizeof(device_driver_t));
    if (driver == NULL) {
        return NULL;
    }
    
    memset(driver, 0, sizeof(device_driver_t));
    
    driver->type = type;
    driver->open = open;
    driver->close = close;
    driver->read = read;
    driver->write = write;
    driver->ioctl = ioctl;
    
    // Copy driver name
    strncpy(driver->name, name, MAX_DEVICE_NAME - 1);
    driver->name[MAX_DEVICE_NAME - 1] = '\0';
    
    return driver;
}