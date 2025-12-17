/*
 * UltraOS Core Types and Data Structures
 * Author: zeroday
 * Date: 2025-12-17
 * Version: 2.0
 */

#ifndef __ULTRAOS_TYPES_H
#define __ULTRAOS_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* Architecture and Device Classification */
typedef enum {
    ARCH_X86_64 = 0,
    ARCH_ARM64,
    ARCH_RISCV,
    ARCH_POWERPC,
    ARCH_MIPS
} architecture_id_t;

typedef enum {
    DEVICE_IOT = 0,        // 128MB - 1GB RAM
    DEVICE_EMBEDDED,       // 1GB - 4GB RAM
    DEVICE_DESKTOP,        // 4GB - 32GB RAM
    DEVICE_SERVER,         // 32GB - 256GB RAM
    DEVICE_WORKSTATION,    // 256GB - 1TB RAM
    DEVICE_SUPERCOMPUTER   // 1TB+ RAM
} device_class_t;

typedef enum {
    GPU_VENDOR_NVIDIA = 0,
    GPU_VENDOR_AMD,
    GPU_VENDOR_INTEL,
    GPU_VENDOR_ARM_MALI,
    GPU_VENDOR_POWERVR,
    GPU_VENDOR_NONE
} gpu_vendor_t;

/* Basic Types */
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uintptr_t usize;
typedef intptr_t  isize;

/* Status Codes */
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR = -1,
    STATUS_NOT_SUPPORTED = -2,
    STATUS_TIMEOUT = -3,
    STATUS_OUT_OF_MEMORY = -4,
    STATUS_INVALID_PARAMETER = -5,
    STATUS_DEVICE_NOT_FOUND = -6,
    STATUS_PERMISSION_DENIED = -7,
    STATUS_ALREADY_INITIALIZED = -8,
    STATUS_NOT_INITIALIZED = -9
} status_t;

/* Hardware Capability Structure */
typedef struct hal_capability {
    architecture_id_t architecture;
    uint64_t memory_capacity;
    uint32_t cpu_cores;
    uint32_t cpu_threads_per_core;
    bool has_gpu;
    gpu_vendor_t gpu_vendor;
    uint64_t gpu_memory;
    uint32_t storage_types;
    bool has_accelerators;
    uint32_t accelerator_types;
    device_class_t device_class;
    
    /* CPU Features */
    bool cpu_features_sse;
    bool cpu_features_sse2;
    bool cpu_features_avx;
    bool cpu_features_avx2;
    bool cpu_features_bmi1;
    bool cpu_features_bmi2;
    bool cpu_features_popcnt;
    uint32_t cpu_logical_cores;
    uint64_t cpu_features_edx;
    uint64_t cpu_features_ecx;
    
    /* System Information */
    uint32_t network_interfaces;
    uint32_t storage_devices;
    char cpu_model_name[64];
    char architecture_name[32];
} hal_capability_t;

/* Device Class Information */
typedef struct device_class_info {
    device_class_t class;
    const char *name;
    uint64_t min_memory_mb;
    uint64_t max_memory_mb;
    uint32_t expected_boot_time_ms;
    uint32_t expected_footprint_mb;
} device_class_info_t;

/* List Structures */
struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

#define LIST_HEAD(name) \
    struct list_head name = { &(name), &(name) }

#define INIT_LIST_HEAD(ptr) do { \
    (ptr)->next = (ptr); \
    (ptr)->prev = (ptr); \
} while (0)

#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

/* Memory Management */
typedef struct memory_zone {
    uint64_t start;
    uint64_t size;
    uint32_t type;
    bool is_available;
    struct list_head list;
} memory_zone_t;

#define MEM_ZONE_NORMAL     0x00000001
#define MEM_ZONE_HIGH       0x00000002
#define MEM_ZONE_DMA        0x00000004
#define MEM_ZONE_DEVICE     0x00000008
#define MEM_ZONE_IOT        0x00000010
#define MEM_ZONE_DESKTOP    0x00000020
#define MEM_ZONE_SERVER     0x00000040
#define MEM_ZONE_HPC        0x00000080

/* CPU Information */
typedef struct cpu_info {
    uint32_t id;
    architecture_id_t arch;
    uint32_t family;
    uint32_t model;
    uint32_t stepping;
    char vendor[16];
    char model_name[64];
    uint32_t features[4];
    uint32_t cache_line_size;
    uint32_t cache_sizes[3]; // L1, L2, L3
    uint32_t frequency_mhz;
} cpu_info_t;

/* Process and Thread Management */
typedef enum {
    STATE_CREATED = 0,
    STATE_READY,
    STATE_RUNNING,
    STATE_WAITING,
    STATE_SLEEPING,
    STATE_TERMINATED
} thread_state_t;

typedef enum {
    PRIORITY_REALTIME = 0,
    PRIORITY_HIGH,
    PRIORITY_NORMAL,
    PRIORITY_LOW,
    PRIORITY_IDLE
} thread_priority_t;

typedef struct thread {
    uint32_t tid;
    uint32_t pid;
    char name[32];
    thread_state_t state;
    thread_priority_t priority;
    void *entry_point;
    void *arg;
    uint64_t time_slice_used;
    uint64_t total_runtime;
    bool preemptible;
    uint32_t cpu_affinity;
    uint64_t stack_base;
    uint64_t stack_size;
    uint64_t stack_pointer;
    struct context_frame {
        uint64_t rax, rbx, rcx, rdx, rsi, rdi;
        uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
        uint64_t rbp, rsp, rip;
        uint64_t rflags;
    } context;
    struct list_head list;
    struct thread *next;
    struct thread *prev;
} thread_t;

typedef struct process {
    uint32_t pid;
    char name[32];
    thread_priority_t priority;
    uint32_t num_threads;
    bool terminated;
    thread_t *main_thread;
    thread_t *threads;
    struct process *next;
    struct process *prev;
} process_t;

/* Scheduler Configuration */
typedef struct scheduler_config {
    uint32_t time_quantum_ms;
    uint32_t max_processes;
    uint32_t max_threads_per_process;
    bool preemptive;
    uint32_t scheduler_algorithm; // 0=RR, 1=Priority, 2=CFS, 3=Multi-queue
} scheduler_config_t;

typedef struct scheduler_stats {
    uint32_t active_processes;
    uint32_t active_threads;
    uint64_t total_context_switches;
    uint64_t total_scheduled_threads;
    uint64_t uptime_ms;
} scheduler_stats_t;

#define MAX_THREADS_PER_PROCESS 1024

/* Performance Monitoring */
typedef struct perf_counter {
    const char *name;
    uint64_t value;
    uint64_t target;
    bool enabled;
} perf_counter_t;

typedef struct boot_metrics {
    uint64_t total_boot_time_ns;
    uint64_t hal_detection_time_ns;
    uint64_t memory_init_time_ns;
    uint64_t module_loading_time_ns;
    uint64_t nexus_init_time_ns;
    bool targets_met;
} boot_metrics_t;

/* Module System */
typedef struct module_requirement {
    uint32_t min_memory_mb;
    uint32_t min_cpu_cores;
    bool require_gpu;
    uint32_t required_architectures;
    uint32_t optional_features;
} module_requirement_t;

typedef struct ultraos_module {
    const char *name;
    const char *description;
    module_requirement_t *requirements;
    int (*init)(void);
    void (*exit)(void);
    uint32_t priority;
    bool loaded;
    struct list_head list;
} ultraos_module_t;

/* Boot Context */
typedef struct boot_context {
    hal_capability_t *hardware_cap;
    device_class_t device_class;
    uint32_t boot_stage;
    bool minimal_mode;
    uint64_t boot_start_time;
    boot_metrics_t metrics;
} boot_context_t;

/* Boot Stages */
#define BOOT_STAGE_HAL_DETECT     0
#define BOOT_STAGE_CORE_INIT      1
#define BOOT_STAGE_MEMORY_INIT    2
#define BOOT_STAGE_MODULE_LOAD    3
#define BOOT_STAGE_SERVICE_START  4
#define BOOT_STAGE_NEXUS_READY    5

/* HAL Types */
typedef enum {
    HAL_SUCCESS = 0,
    HAL_ERROR = -1,
    HAL_NOT_FOUND = -2,
    HAL_NOT_SUPPORTED = -3
} hal_init_t;

typedef struct hal_driver {
    const char *name;
    const char *description;
    uint32_t type;
    int (*init)(void);
    int (*probe)(void);
    void (*shutdown)(void);
    struct list_head list;
} hal_driver_t;

typedef struct hal_device {
    const char *name;
    uint32_t type;
    void *private_data;
    struct list_head list;
} hal_device_t;

/* Power Management */
typedef enum {
    POWER_ON = 0,
    POWER_STANDBY,
    POWER_SUSPEND,
    POWER_HIBERNATE,
    POWER_OFF
} hal_power_state_t;

typedef struct hal_power_info {
    hal_power_state_t current_state;
    uint32_t battery_level;
    bool ac_connected;
    uint32_t estimated_remaining_time;
} hal_power_info_t;

/* Security Types */
typedef enum {
    SECURITY_LEVEL_NONE = 0,
    SECURITY_LEVEL_BASIC,
    SECURITY_LEVEL_STANDARD,
    SECURITY_LEVEL_HIGH,
    SECURITY_LEVEL_MAXIMUM
} security_level_t;

typedef struct security_context {
    security_level_t level;
    bool encryption_enabled;
    bool isolation_enabled;
    bool verified_boot;
} security_context_t;

/* Utility Macros */
#define ALIGN_UP(x, align)      (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align)    ((x) & ~((align) - 1))
#define IS_ALIGNED(x, align)    (((x) & ((align) - 1)) == 0)
#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))
#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/* Debug Macros */
#if ULTRAOS_DEBUG
#define ULTRAOS_DEBUG_PRINT(fmt, ...) printk("[DEBUG] %s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define ASSERT(cond) do { \
    if (!(cond)) { \
        printk("[ASSERT] %s:%d: %s", __func__, __LINE__, #cond); \
        while(1) { __asm__ volatile("hlt"); } \
    } \
} while(0)
#else
#define ULTRAOS_DEBUG_PRINT(fmt, ...) do { } while(0)
#define ASSERT(cond) do { } while(0)
#endif

/* Memory Allocation Flags */
#define GFP_KERNEL    0x00000001
#define GFP_ATOMIC    0x00000002
#define GFP_USER      0x00000004
#define GFP_HIGHUSER  0x00000008

/* String and I/O functions */
int printk(const char *fmt, ...);
int puts(const char *s);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

/* List manipulation functions */
void list_add(struct list_head *new, struct list_head *head);
void list_del(struct list_head *entry);
bool list_empty(struct list_head *head);

/* Forward declarations */
extern boot_context_t *current_boot_context;
extern bool kernel_initialized;
extern boot_metrics_t boot_metrics;

/* Include configuration */
#include "ultraos_config.h"

#endif /* __ULTRAOS_TYPES_H */