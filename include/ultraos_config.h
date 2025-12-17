/*
 * UltraOS Configuration Header
 * Author: zeroday
 * Date: 2025-12-17
 * Version: 2.0
 */

#ifndef __ULTRAOS_CONFIG_H
#define __ULTRAOS_CONFIG_H

/* Architecture Configuration */
#if defined(__x86_64__) || defined(__x86_64)
    #define ULTRAOS_ARCH_X86_64 1
    #define ULTRAOS_ARCH_BITS 64
    #define ULTRAOS_LITTLE_ENDIAN 1
#elif defined(__aarch64__) || defined(__arm64__)
    #define ULTRAOS_ARCH_ARM64 1
    #define ULTRAOS_ARCH_BITS 64
    #define ULTRAOS_LITTLE_ENDIAN 1
#elif defined(__riscv) && __riscv_xlen == 64
    #define ULTRAOS_ARCH_RISCV 1
    #define ULTRAOS_ARCH_BITS 64
    #define ULTRAOS_LITTLE_ENDIAN 1
#elif defined(__powerpc64__) || defined(__ppc64__)
    #define ULTRAOS_ARCH_POWERPC 1
    #define ULTRAOS_ARCH_BITS 64
    #define ULTRAOS_BIG_ENDIAN 1
#elif defined(__mips__) && defined(__mips64)
    #define ULTRAOS_ARCH_MIPS 1
    #define ULTRAOS_ARCH_BITS 64
    #define ULTRAOS_BIG_ENDIAN 1
#else
    #error "Unsupported architecture"
#endif

/* Build Configuration */
#ifndef ULTRAOS_VERSION
    #define ULTRAOS_VERSION "2.0.0"
#endif

#ifndef ULTRAOS_VERSION_CODE
    #define ULTRAOS_VERSION_CODE 200
#endif

/* Feature Flags */
#ifndef ULTRAOS_AI
    #define ULTRAOS_AI 0
#endif

#ifndef ULTRAOS_QUANTUM
    #define ULTRAOS_QUANTUM 0
#endif

#ifndef ULTRAOS_SECURITY
    #define ULTRAOS_SECURITY 0
#endif

/* Core Features */
#define ULTRAOS_SMP 1
#define ULTRAOS_DEBUG 0
#define ULTRAOS_COMPRESSED_KERNEL 1
#define ULTRAOS_MODULAR 1
#define ULTRAOS_HAL 1

/* Performance Targets */
#define ULTRAOS_IOT_BOOT_TIME_TARGET_NS 1000000000ULL      /* 1 second */
#define ULTRAOS_DESKTOP_BOOT_TIME_TARGET_NS 2000000000ULL  /* 2 seconds */
#define ULTRAOS_SERVER_BOOT_TIME_TARGET_NS 5000000000ULL   /* 5 seconds */
#define ULTRAOS_HPC_BOOT_TIME_TARGET_NS 10000000000ULL     /* 10 seconds */

/* Memory Management */
#define ULTRAOS_DEFAULT_PAGE_SIZE 4096
#define ULTRAOS_HUGEPAGE_SIZE 2097152
#define ULTRAOS_GIANTPAGE_SIZE 1073741824
#define ULTRAOS_KERNEL_STACK_SIZE 8192
#define ULTRAOS_MAX_MEMORY_ZONES 32

/* Scheduler Configuration */
#define ULTRAOS_DEFAULT_TIME_QUANTUM_MS 10
#define ULTRAOS_MAX_PROCESSES 1024
#define ULTRAOS_MAX_THREADS_PER_PROCESS 1024
#define ULTRAOS_MAX_PRIORITY_LEVELS 32

/* Module System */
#define ULTRAOS_MODULE_LOAD_TIMEOUT_US 100000   /* 100ms */
#define ULTRAOS_MAX_MODULES 256
#define ULTRAOS_MODULE_NAME_MAX 64

/* Performance Monitoring */
#define ULTRAOS_BOOT_PERF_MONITORING 1
#define ULTRAOS_MEMORY_PERF_MONITORING 1
#define ULTRAOS_MODULE_PERF_MONITORING 1
#define ULTRAOS_SCHEDULER_PERF_MONITORING 1

/* Security Configuration */
#define ULTRAOS_SECURITY_LEVEL STANDARD
#define ULTRAOS_ENABLE_SECURE_BOOT 1
#define ULTRAOS_ENABLE_MEMORY_PROTECTION 1
#define ULTRAOS_ENABLE_STACK_PROTECTION 1
#define ULTRAOS_ENABLE_ASLR 1

/* HAL Configuration */
#define ULTRAOS_MAX_HAL_DEVICES 256
#define ULTRAOS_MAX_HAL_DRIVERS 64
#define ULTRAOS_HAL_TIMEOUT_MS 5000

/* Network Configuration */
#define ULTRAOS_MAX_NETWORK_INTERFACES 16
#define ULTRAOS_NETWORK_BUFFER_SIZE 65536
#define ULTRAOS_MAX_CONNECTIONS 1024

/* Storage Configuration */
#define ULTRAOS_MAX_STORAGE_DEVICES 32
#define ULTRAOS_MAX_FILESYSTEMS 16
#define ULTRAOS_BUFFER_CACHE_SIZE (64 * 1024 * 1024)

/* Interrupt Configuration */
#define ULTRAOS_MAX_INTERRUPTS 256
#define ULTRAOS_INTERRUPT_STACK_SIZE 4096

/* Timer Configuration */
#define ULTRAOS_TIMER_FREQUENCY 1000  /* Hz */
#define ULTRAOS_MAX_TIMERS 256

/* Console Configuration */
#define ULTRAOS_CONSOLE_BUFFER_SIZE 4096
#define ULTRAOS_MAX_CONSOLES 4

/* Debug Configuration */
#define ULTRAOS_DEBUG_BUFFER_SIZE (1 * 1024 * 1024)
#define ULTRAOS_MAX_DEBUG_MESSAGES 8192
#define ULTRAOS_ENABLE_DEBUG_SYMBOLS 0

/* Feature-specific Configuration */
#if ULTRAOS_AI
    #define ULTRAOS_MAX_AI_ACCELERATORS 8
    #define ULTRAOS_AI_TENSOR_MAX_DIMENSIONS 8
    #define ULTRAOS_AI_MODEL_MAX_SIZE (256 * 1024 * 1024)  /* 256MB */
#endif

#if ULTRAOS_QUANTUM
    #define ULTRAOS_MAX_QUBITS 1024
    #define ULTRAOS_MAX_QUANTUM_CIRCUITS 64
    #define ULTRAOS_QUANTUM_SIMULATION_MAX_QUBITS 32
#endif

#if ULTRAOS_SECURITY
    #define ULTRAOS_MAX_KEY_SIZE 4096
    #define ULTRAOS_MAX_CERTIFICATES 256
    #define ULTRAOS_SECURITY_AUDIT_LOG_SIZE (16 * 1024 * 1024)
#endif

/* Power Management */
#define ULTRAOS_POWER_MANAGEMENT 1
#define ULTRAOS_MAX_POWER_STATES 8
#define ULTRAOS_IDLE_TIMEOUT_MS 60000  /* 1 minute */

/* Virtual Memory */
#define ULTRAOS_VIRTUAL_MEMORY 1
#define ULTRAOS_MAX_VIRTUAL_ADDRESS_SPACE (1ULL << 48)
#define ULTRAOS_MAX_PAGE_TABLES 1024

/* SMP Configuration */
#if ULTRAOS_SMP
    #define ULTRAOS_MAX_CPUS 256
    #define ULTRAOS_CPU_LOCAL_STORAGE_SIZE 4096
#endif

/* Development Configuration */
#define ULTRAOS_ENABLE_UNIT_TESTS 1
#define ULTRAOS_ENABLE_INTEGRATION_TESTS 1
#define ULTRAOS_ENABLE_BENCHMARKS 1
#define ULTRAOS_COVERAGE_THRESHOLD 80

/* Build-time Optimizations */
#define ULTRAOS_OPTIMIZE_FOR_SIZE 0
#define ULTRAOS_OPTIMIZE_FOR_SPEED 1
#define ULTRAOS_ENABLE_LTO 0
#define ULTRAOS_ENABLE_PGO 0

/* Compatibility */
#define ULTRAOS_POSIX_COMPAT 1
#define ULTRAOS_LINUX_COMPAT 1
#define ULTRAOS_WINDOWS_COMPAT 0

/* Memory Alignment */
#define ULTRAOS_ALIGNMENT 16
#define ULTRAOS_CACHE_LINE_SIZE 64
#define ULTRAOS_PAGE_ALIGN 0x1000

/* Error Handling */
#define ULTRAOS_PANIC_ON_CRITICAL_ERROR 1
#define ULTRAOS_RECOVERY_MODE 1
#define ULTRAOS_SAFE_MODE 1

/* Logging Configuration */
#define ULTRAOS_LOG_LEVEL INFO
#define ULTRAOS_MAX_LOG_SIZE (8 * 1024 * 1024)
#define ULTRAOS_LOG_ROTATION_COUNT 4

/* Development Macros */
#define ULTRAOS_UNIMPLEMENTED() do { \
    printk("UNIMPLEMENTED: %s at %s:%d\n", __func__, __FILE__, __LINE__); \
    while(1) { __asm__ volatile("hlt"); } \
} while(0)

#define ULTRAOS_UNUSED(x) ((void)(x))

/* Compiler-specific optimizations */
#if defined(__GNUC__)
    #define ULTRAOS_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define ULTRAOS_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define ULTRAOS_INLINE      __inline__ __attribute__((always_inline))
    #define ULTRAOS_NOINLINE    __attribute__((noinline))
    #define ULTRAOS_PACKED      __attribute__((packed))
    #define ULTRAOS_ALIGNED(x)  __attribute__((aligned(x)))
#else
    #define ULTRAOS_LIKELY(x)   (x)
    #define ULTRAOS_UNLIKELY(x) (x)
    #define ULTRAOS_INLINE      inline
    #define ULTRAOS_NOINLINE
    #define ULTRAOS_PACKED
    #define ULTRAOS_ALIGNED(x)
#endif

/* Validation macros */
#define ULTRAOS_VALIDATE_PTR(ptr) \
    do { \
        if (!ptr) return STATUS_INVALID_PARAMETER; \
    } while(0)

#define ULTRAOS_VALIDATE_RANGE(ptr, size) \
    do { \
        if (!ptr || size == 0) return STATUS_INVALID_PARAMETER; \
    } while(0)

#define ULTRAOS_VALIDATE_STATE(condition) \
    do { \
        if (!(condition)) return STATUS_ERROR; \
    } while(0)

#endif /* __ULTRAOS_CONFIG_H */