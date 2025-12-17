# UltraOS API Documentation

## Overview

UltraOS provides a comprehensive set of APIs for system programming, hardware abstraction, and application development. This document covers the core APIs available in the UltraOS kernel.

## Table of Contents

1. [System Call API](#system-call-api)
2. [Memory Management API](#memory-management-api)
3. [Process Management API](#process-management-api)
4. [Device Management API](#device-management-api)
5. [File System API](#file-system-api)
6. [Network API](#network-api)
7. [Hardware Abstraction Layer](#hardware-abstraction-layer)
8. [Kernel Debugging API](#kernel-debugging-api)

## System Call API

### Core System Calls

#### Process Management
- `syscall_fork()` - Create a new process
- `syscall_execve()` - Execute a new program
- `syscall_exit()` - Terminate current process
- `syscall_getpid()` - Get current process ID
- `syscall_getppid()` - Get parent process ID
- `syscall_kill()` - Send signal to process

#### File Operations
- `syscall_open()` - Open a file
- `syscall_close()` - Close a file descriptor
- `syscall_read()` - Read from file
- `syscall_write()` - Write to file
- `syscall_lseek()` - Seek in file
- `syscall_stat()` - Get file status
- `syscall_fstat()` - Get file status by descriptor

#### Memory Management
- `syscall_mmap()` - Map memory region
- `syscall_munmap()` - Unmap memory region
- `syscall_mprotect()` - Change memory protection

#### Signal Handling
- `syscall_sigaction()` - Set signal handler
- `syscall_sigprocmask()` - Examine and change signal mask
- `syscall_sigsuspend()` - Wait for signal

#### Process Control
- `syscall_prctl()` - Process control operations

### Usage Examples

```c
#include <ultraos_syscalls.h>

// Fork a new process
pid_t pid = syscall_fork();
if (pid == 0) {
    // Child process
    const char *args[] = {"/bin/ls", "-l", "/", NULL};
    syscall_execve("/bin/ls", args, NULL);
} else if (pid > 0) {
    // Parent process
    int status;
    syscall_waitpid(pid, &status, 0);
}

// Open and read a file
int fd = syscall_open("/etc/hosts", O_RDONLY, 0);
if (fd >= 0) {
    char buffer[1024];
    ssize_t n = syscall_read(fd, buffer, sizeof(buffer));
    if (n > 0) {
        buffer[n] = '\0';
        printf("File content: %s\n", buffer);
    }
    syscall_close(fd);
}

// Map memory
void *addr = syscall_mmap(NULL, 4096, PROT_READ | PROT_WRITE, 
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
if (addr != MAP_FAILED) {
    // Use mapped memory
    strcpy((char*)addr, "Hello, UltraOS!");
    syscall_munmap(addr, 4096);
}
```

## Memory Management API

### Physical Memory Management

#### Memory Allocation
```c
// Allocate physical memory page
void* phys_alloc_page(void);

// Free physical memory page
void phys_free_page(void* page);

// Allocate physical memory pages
void* phys_alloc_pages(size_t count);

// Free physical memory pages
void phys_free_pages(void* addr, size_t count);
```

#### Memory Mapping
```c
// Map physical address to virtual address
status_t phys_map_virtual(void* virt_addr, void* phys_addr, 
                         size_t size, uint32_t flags);

// Unmap virtual address
status_t phys_unmap_virtual(void* virt_addr, size_t size);

// Get physical address from virtual address
void* virt_to_phys(void* virt_addr);

// Get virtual address from physical address
void* phys_to_virt(void* phys_addr);
```

### Virtual Memory Management

#### Virtual Memory Allocation
```c
// Allocate virtual memory region
void* vm_alloc_region(size_t size, uint32_t flags);

// Free virtual memory region
status_t vm_free_region(void* addr, size_t size);

// Change memory protection
status_t vm_protect(void* addr, size_t size, uint32_t protection);

// Get memory statistics
vm_stats_t* vm_get_stats(void);
```

#### Page Table Management
```c
// Create new page table
page_table_t* pt_create(void);

// Destroy page table
void pt_destroy(page_table_t* pt);

// Map page in page table
status_t pt_map_page(page_table_t* pt, uintptr_t virt_addr, 
                    uintptr_t phys_addr, uint32_t flags);

// Unmap page from page table
status_t pt_unmap_page(page_table_t* pt, uintptr_t virt_addr);
```

## Process Management API

### Process Creation and Management

#### Process Control Block
```c
// Get current process
process_t* process_get_current(void);

// Get process by PID
process_t* process_get_by_pid(pid_t pid);

// Create new process
process_t* process_create(const char* name, uint32_t flags);

// Terminate process
void process_terminate(process_t* process, int exit_code);
```

#### Thread Management
```c
// Create new thread
thread_t* thread_create(process_t* process, thread_func_t func, 
                       void* arg, uint32_t flags);

// Terminate thread
void thread_terminate(thread_t* thread);

// Get current thread
thread_t* thread_get_current(void);

// Thread synchronization primitives
status_t mutex_init(mutex_t* mutex);
status_t mutex_lock(mutex_t* mutex);
status_t mutex_unlock(mutex_t* mutex);
status_t mutex_destroy(mutex_t* mutex);
```

### Scheduler API

#### Scheduling Control
```c
// Initialize scheduler
status_t scheduler_init(void);

// Add process to scheduler
status_t scheduler_add_process(process_t* process);

// Remove process from scheduler
status_t scheduler_remove_process(process_t* process);

// Yield CPU
void scheduler_yield(void);

// Get scheduler statistics
scheduler_stats_t* scheduler_get_stats(void);
```

## Device Management API

### Device Registration

#### Device Drivers
```c
// Create device driver
device_driver_t* devices_create_driver(device_type_t type, const char* name,
                                       device_open_t open, device_close_t close,
                                       device_read_t read, device_write_t write,
                                       device_ioctl_t ioctl);

// Register device driver
status_t devices_register_driver(device_driver_t* driver);

// Create device
status_t devices_create_device(device_type_t type, const char* name, 
                               device_driver_t* driver, void* data);
```

### Device Operations

#### Opening and Closing Devices
```c
// Open device
device_t* devices_open(const char* name, uint32_t flags);

// Close device
status_t devices_close(device_t* device);
```

#### Device I/O Operations
```c
// Read from device
int64_t devices_read(device_t* device, void* buf, uint64_t count);

// Write to device
int64_t devices_write(device_t* device, const void* buf, uint64_t count);

// Device control operations
int64_t devices_ioctl(device_t* device, uint32_t cmd, uint64_t arg);
```

### Device Types

#### Character Devices
```c
// Create character device
status_t devices_create_chardev(const char* name, device_driver_t* driver, 
                                void* data);

// Character device driver example
device_driver_t* create_serial_driver(void) {
    return devices_create_driver(DEVICE_TYPE_CHAR, "serial",
                                serial_open, serial_close,
                                serial_read, serial_write,
                                serial_ioctl);
}
```

#### Block Devices
```c
// Create block device
status_t devices_create_blockdev(const char* name, device_driver_t* driver, 
                                 void* data);

// Block device driver example
device_driver_t* create_disk_driver(void) {
    return devices_create_driver(DEVICE_TYPE_BLOCK, "disk",
                                disk_open, disk_close,
                                disk_read, disk_write,
                                disk_ioctl);
}
```

## File System API

### File System Operations

#### Mounting and Unmounting
```c
// Mount file system
status_t fs_mount(const char* device, const char* mountpoint, 
                 fs_type_t type, uint32_t flags);

// Unmount file system
status_t fs_unmount(const char* mountpoint);

// List mount points
status_t fs_list_mounts(void);
```

#### File Operations
```c
// Open file
file_t* fs_open(const char* path, uint32_t flags, uint16_t mode);

// Close file
status_t fs_close(file_t* file);

// Read from file
int64_t fs_read(file_t* file, void* buf, uint64_t count);

// Write to file
int64_t fs_write(file_t* file, const void* buf, uint64_t count);

// Seek in file
int64_t fs_lseek(file_t* file, int64_t offset, int32_t whence);
```

#### Directory Operations
```c
// Create directory
status_t fs_mkdir(const char* path, uint16_t mode);

// Remove directory
status_t fs_rmdir(const char* path);

// Read directory
int64_t fs_readdir(const char* path, dir_entry_t* entries, 
                  uint32_t max_entries);

// Remove file
status_t fs_unlink(const char* path);

// Rename file
status_t fs_rename(const char* old_path, const char* new_path);
```

### File System Types

#### Built-in File Systems
```c
// Available file system types
FS_TYPE_TMPFS    // Temporary file system (RAM-based)
FS_TYPE_DEVFS    // Device file system
FS_TYPE_PROCFS   // Process file system
```

#### Custom File Systems
```c
// Register custom file system
status_t fs_register_type(fs_type_t type, const char* name,
                          fs_mount_func_t mount_func, 
                          fs_unmount_func_t unmount_func);
```

## Network API

### Network Interface Management

#### Interface Operations
```c
// Register network interface
status_t net_register_interface(net_interface_t* iface);

// Get interface by name
net_interface_t* net_get_interface(const char* name);

// List all interfaces
status_t net_list_interfaces(void);
```

#### Packet Handling
```c
// Create network packet
net_packet_t* net_create_packet(uint16_t protocol, uint16_t length);

// Send packet
status_t net_send_packet(net_interface_t* iface, net_packet_t* packet);

// Free packet
status_t net_free_packet(net_packet_t* packet);
```

#### Protocol Handling
```c
// Register protocol handler
status_t net_register_protocol(uint16_t protocol, protocol_handler_t handler);
```

### Network Buffer Management

#### Buffer Allocation
```c
// Allocate network buffer
net_buf_t* net_alloc_buffer(void);

// Free network buffer
status_t net_free_buffer(net_buf_t* buf);
```

## Hardware Abstraction Layer

### CPU Operations

#### CPU Information
```c
// Get CPU information
cpu_info_t* cpu_get_info(void);

// Get CPU ID
uint32_t cpu_get_id(void);

// Get CPU frequency
uint64_t cpu_get_frequency(void);
```

#### CPU Control
```c
// Enable interrupts
void interrupts_enable(void);

// Disable interrupts
void interrupts_disable(void);

// Send End of Interrupt
void interrupts_send_eoi(uint8_t irq);

// Get CPU statistics
cpu_stats_t* cpu_get_stats(void);
```

### Memory Management Unit (MMU)

#### MMU Operations
```c
// Enable MMU
status_t mmu_enable(void);

// Disable MMU
status_t mmu_disable(void);

// Flush TLB
void mmu_flush_tlb(void);

// Invalidate TLB entry
void mmu_invalidate_tlb(uintptr_t virt_addr);
```

### Power Management

#### System Power Control
```c
// Suspend system
status_t power_suspend(void);

// Resume system
status_t power_resume(void);

// Shutdown system
void power_shutdown(void);

// Reboot system
void power_reboot(void);
```

## Kernel Debugging API

### Debug Output

#### Kernel Messages
```c
// Print kernel message
void printk(const char* format, ...);

// Set log level
void debug_set_level(debug_level_t level);

// Get log statistics
debug_stats_t* debug_get_stats(void);
```

### Memory Debugging

#### Memory Validation
```c
// Validate pointer
bool memory_is_valid(void* ptr);

// Check memory alignment
bool memory_is_aligned(void* ptr, size_t alignment);

// Get memory statistics
memory_stats_t* memory_get_stats(void);
```

### Performance Monitoring

#### Performance Counters
```c
// Start performance counter
void perf_start_counter(perf_counter_t* counter);

// Stop performance counter
uint64_t perf_stop_counter(perf_counter_t* counter);

// Get performance statistics
perf_stats_t* perf_get_stats(void);
```

## Error Handling

### Error Codes

UltraOS uses a comprehensive error code system:

```c
// Success
STATUS_SUCCESS           0

// General errors
STATUS_INVALID_PARAMETER -1
STATUS_OUT_OF_MEMORY     -2
STATUS_NOT_FOUND         -3
STATUS_ALREADY_EXISTS    -4
STATUS_PERMISSION_DENIED -5
STATUS_NOT_SUPPORTED     -6
STATUS_INVALID_STATE     -7

// System call errors (POSIX compatible)
-ENOENT                  -2    // No such file or directory
-EACCES                  -13   // Permission denied
-EBADF                   -9    // Bad file descriptor
-EINVAL                  -22   // Invalid argument
-ENOSYS                  -38   // Function not implemented
-ENOMEM                  -12   // Cannot allocate memory
-EBUSY                   -16   // Device or resource busy
-EIO                     -5    // I/O error
```

### Error Handling Best Practices

```c
#include <ultraos_types.h>

// Proper error handling example
status_t my_operation(void) {
    void* ptr = kmalloc(1024);
    if (ptr == NULL) {
        return STATUS_OUT_OF_MEMORY;
    }
    
    // Perform operation
    status_t ret = perform_operation(ptr);
    
    // Clean up
    kfree(ptr);
    
    return ret;
}

// Using error checking with system calls
int fd = syscall_open("/dev/tty0", O_RDWR, 0);
if (fd < 0) {
    if (fd == -ENOENT) {
        printf("Device not found\n");
    } else if (fd == -EACCES) {
        printf("Permission denied\n");
    } else {
        printf("Error: %d\n", fd);
    }
    return -1;
}

// Safe memory allocation with error checking
void* ptr = kmalloc(size);
if (ptr == NULL) {
    printk("Memory allocation failed for size %zu\n", size);
    return NULL;
}

// Perform operations...
```

## Advanced Topics

### Interrupt Handling

#### Custom Interrupt Handlers
```c
// Register interrupt handler
status_t interrupts_register_irq(uint8_t irq, irq_handler_t handler);

// Example interrupt handler
void my_interrupt_handler(uint8_t irq) {
    printk("Interrupt %u occurred\n", irq);
    
    // Handle interrupt
    // ...
    
    // Send EOI
    interrupts_send_eoi(irq);
}

// Register the handler
interrupts_register_irq(0, my_interrupt_handler);
```

### Synchronization Primitives

#### Mutex Implementation
```c
// Initialize mutex
status_t mutex_init(mutex_t* mutex) {
    if (mutex == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    
    mutex->owner = NULL;
    mutex->count = 0;
    INIT_LIST_HEAD(&mutex->wait_list);
    
    return STATUS_SUCCESS;
}

// Lock mutex
status_t mutex_lock(mutex_t* mutex) {
    thread_t* current = thread_get_current();
    
    if (current == mutex->owner) {
        mutex->count++;
        return STATUS_SUCCESS;
    }
    
    // Add to wait list and block
    list_add_tail(&current->wait_list, &mutex->wait_list);
    thread_block(current, THREAD_STATE_WAITING);
    
    return STATUS_SUCCESS;
}

// Unlock mutex
status_t mutex_unlock(mutex_t* mutex) {
    thread_t* current = thread_get_current();
    
    if (current != mutex->owner) {
        return STATUS_PERMISSION_DENIED;
    }
    
    if (mutex->count > 0) {
        mutex->count--;
        return STATUS_SUCCESS;
    }
    
    // Wake up waiting thread
    thread_t* waiter = list_first_entry(&mutex->wait_list, 
                                        thread_t, wait_list);
    list_del(&waiter->wait_list);
    thread_wakeup(waiter);
    mutex->owner = waiter;
    
    return STATUS_SUCCESS;
}
```

This API documentation provides comprehensive coverage of the UltraOS kernel APIs. For more detailed examples and advanced usage patterns, refer to the source code in the kernel directories.