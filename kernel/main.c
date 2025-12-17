/*
 * UltraOS Main Entry Point
 * Author: zeroday
 * Date: 2025-12-17
 * Version: 2.0
 */

#include "../include/ultraos_types.h"
#include "../include/ultraos_config.h"
#include "core/kernel.h"
#include "hal/hal.h"

/* External assembly functions */
extern void early_setup(void);
extern void main_console_init(void);

/* Simple putchar implementation for basic I/O */
int putchar(int c) {
    /* Architecture-specific output would go here */
    /* For now, just increment a counter and return the character */
    static unsigned int output_count = 0;
    output_count++;
    
    /* Simple console output for development */
    #if defined(__x86_64__) || defined(__x86_64)
        /* x86_64 serial output (simplified) */
        if (c == '\n') {
            /* Handle newline */
        }
    #endif
    
    return c;
}

/* Entry point for the kernel */
void ultraos_main(void) {
    /* Initialize early console for debug output */
    main_console_init();
    
    printk("=== UltraOS Kernel Starting ===\n");
    printk("Version: %s\n", ULTRAOS_VERSION);
    printk("Build: %s\n", 
           #if ULTRAOS_DEBUG
           "Debug"
           #else
           "Release"
           #endif
    );
    printk("Architecture: x86_64\n");
    
    /* Perform early setup */
    early_setup();
    
    /* Start the boot sequence */
    int result = ultraos_boot();
    
    if (result != STATUS_SUCCESS) {
        printk("Boot failed with error: %d\n", result);
        panic("Boot sequence failed");
    }
    
    printk("=== UltraOS Boot Complete ===\n");
    printk("Kernel is now running...\n");
    
    /* Main kernel loop */
    while (1) {
        /* This would be the main kernel loop handling:
         * - Process scheduling
         * - Timer interrupts
         * - System calls
         * - Device interrupts
         * - Power management
         */
        
        /* For now, just a simple delay */
        for (volatile int i = 0; i < 1000000; i++) {
            /* Busy wait */
        }
        
        printk("Kernel tick...\n");
        break; /* Exit after one cycle for demo */
    }
    
    /* We should never reach here in a real system */
    printk("Kernel main loop exited - this should not happen!\n");
    ultraos_shutdown();
}

/* Boot function that calls the main entry point */
void boot(void) {
    ultraos_main();
}

/* Early console initialization for debug output */
void main_console_init(void) {
    /* Initialize serial console for early debug output */
    #if defined(__x86_64__) || defined(__x86_64)
        /* Initialize x86_64 serial port (COM1: 0x3F8) */
        /* This is a simplified initialization for development */
        __asm__ volatile(
            "mov $0x3F8, %%dx\n"      /* COM1 port */
            "mov $0x00, %%al\n"       /* Disable interrupts */
            "out %%al, %%dx\n"
            "mov $0x80, %%al\n"       /* Set baud rate divisor */
            "out %%al, %%dx\n"
            "mov $0x03, %%al\n"       /* 8N1, enable DLAB */
            "out %%al, %%dx\n"
            "mov $0x00, %%al\n"       /* Baud rate low byte */
            "out %%al, %%dx\n"
            "mov $0x00, %%al\n"       /* Baud rate high byte */
            "out %%al, %%dx\n"
            "mov $0x03, %%al\n"       /* 8N1, disable DLAB */
            "out %%al, %%dx\n"
            "mov $0x01, %%al\n"       /* Enable interrupts */
            "out %%al, %%dx\n"
            :
            :
            : "dx", "al", "memory"
        );
    #endif
}

/* Simple heap allocation (very basic, for testing) */
static char heap[1024 * 1024];  /* 1MB heap */
static size_t heap_top = 0;

/* Basic memory allocation for early boot */
void* kmalloc(size_t size, uint32_t flags) {
    /* Simple allocator doesn't support different flags yet */
    (void)flags;
    
    if (heap_top + size > sizeof(heap)) {
        return NULL;  /* Out of memory */
    }
    
    void *ptr = &heap[heap_top];
    heap_top += ALIGN_UP(size, ULTRAOS_ALIGNMENT);
    
    return ptr;
}

void kfree(void *ptr) {
    /* Simple allocator doesn't support free */
    /* In a real implementation, this would be much more sophisticated */
    (void)ptr;
}

/* List manipulation functions */
void list_add(struct list_head *new, struct list_head *head) {
    new->next = head;
    new->prev = head->prev;
    head->prev->next = new;
    head->prev = new;
}

void list_del(struct list_head *entry) {
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}

bool list_empty(struct list_head *head) {
    return head->next == head;
}

/* Architecture-specific early setup */
void early_setup(void) {
    /* This would contain architecture-specific early initialization */
    /* such as setting up page tables, GDT, IDT, etc. */
    printk("Early setup completed\n");
}

/* Initialize profiler function pointers */
void init_profiler_functions(void) {
    /* Initialize performance profiling functions */
    /* This would set up function pointers for different profiling subsystems */
}

/* Architecture-specific CPU driver registration */
extern int x86_register_driver(void);
extern int arm64_register_driver(void);
extern int riscv_register_driver(void);
extern int powerpc_register_driver(void);
extern int mips_register_driver(void);

/* Register all built-in drivers */
void register_builtin_drivers(void) {
    printk("Registering built-in drivers...\n");
    
    /* Register architecture-specific drivers */
    #if ULTRAOS_ARCH_X86_64
    x86_register_driver();
    #endif
    #if ULTRAOS_ARCH_ARM64
    arm64_register_driver();
    #endif
    #if ULTRAOS_ARCH_RISCV
    riscv_register_driver();
    #endif
    #if ULTRAOS_ARCH_POWERPC
    powerpc_register_driver();
    #endif
    #if ULTRAOS_ARCH_MIPS
    mips_register_driver();
    #endif
    
    printk("Built-in drivers registered\n");
}

/* Initialize memory allocation (simple) */
void init_memory_system(void) {
    printk("Initializing memory system...\n");
    heap_top = 0;
    memset(heap, 0, sizeof(heap));
    printk("Memory system initialized (heap: %lu bytes)\n", sizeof(heap));
}

/* Get kernel uptime (simplified) */
uint64_t kernel_get_uptime_ns(void) {
    static uint64_t uptime = 0;
    uptime += 10000000; /* 10ms increment for demo */
    return uptime;
}

/* Process creation (simplified) */
int kernel_create_process(const char *name) {
    printk("Creating process: %s\n", name);
    return 0;
}

/* Thread creation (simplified) */
int kernel_create_thread(uint32_t pid, const char *name, void (*start_func)(void)) {
    printk("Creating thread: %s (PID: %u)\n", name, pid);
    return 0;
}

/* Simple timer implementation */
uint64_t current_time = 0;

uint64_t kernel_get_time_ns(void) {
    return current_time += 1000000; /* 1ms increment for demo */
}

uint64_t kernel_get_time_us(void) {
    return current_time / 1000;
}

void kernel_delay_ns(uint64_t ns) {
    /* Simple busy-wait delay */
    volatile uint64_t end = current_time + ns;
    while (current_time < end) {
        /* Busy wait */
    }
}

void kernel_delay_us(uint64_t us) {
    kernel_delay_ns(us * 1000);
}

/* Set up the basic data structures for the kernel */
void kernel_early_init(void) {
    printk("Kernel early initialization...\n");
    
    /* Initialize lists */
    /* This would initialize all the kernel data structures */
    
    printk("Kernel early initialization complete\n");
}

/* Test function for scheduler */
void test_scheduler(void) {
    printk("Testing scheduler functionality...\n");
    
    /* Create a test process */
    pid_t pid = scheduler_create_process("test_process", (void*)0x1000, PRIORITY_NORMAL);
    if (pid > 0) {
        printk("Created test process with PID: %d\n", pid);
        
        /* Create a test thread */
        tid_t tid = scheduler_create_thread(pid, "test_thread", (void*)0x2000, NULL, PRIORITY_NORMAL);
        if (tid > 0) {
            printk("Created test thread with TID: %d\n", tid);
            
            /* Run scheduler */
            scheduler_start();
            printk("Scheduler started\n");
            
            /* Test scheduling */
            for (int i = 0; i < 10; i++) {
                scheduler_schedule();
                printk("Scheduled cycle %d\n", i + 1);
            }
            
            printk("Scheduler test completed\n");
        } else {
            printk("Failed to create test thread\n");
        }
    } else {
        printk("Failed to create test process\n");
    }
}

/* Debug function to print system state */
void debug_print_system_state(void) {
    printk("=== System State ===\n");
    printk("Kernel initialized: %s\n", kernel_is_initialized() ? "Yes" : "No");
    
    boot_context_t *ctx = kernel_get_boot_context();
    if (ctx && ctx->hardware_cap) {
        printk("Device class: %s\n", device_class_name(ctx->device_class));
        printk("Memory capacity: %lu MB\n", ctx->hardware_cap->memory_capacity / (1024 * 1024));
        printk("CPU cores: %u\n", ctx->hardware_cap->cpu_cores);
    }
    
    scheduler_stats_t stats = scheduler_get_stats();
    printk("Active processes: %u\n", stats.active_processes);
    printk("Active threads: %u\n", stats.active_threads);
    printk("Context switches: %lu\n", stats.total_context_switches);
    printk("===================\n");
}