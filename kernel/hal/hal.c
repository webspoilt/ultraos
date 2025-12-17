/*
 * UltraOS Hardware Abstraction Layer (HAL)
 * Author: zeroday
 * Date: 2025-12-17
 * Version: 2.0
 */

#include "../../include/ultraos_types.h"
#include "../../include/ultraos_config.h"

/* HAL global state */
static struct {
    bool initialized;
    hal_capability_t capabilities;
    LIST_HEAD(driver_list);
    LIST_HEAD(device_list);
    uint32_t num_drivers;
    uint32_t num_devices;
} hal_state;

/* Forward declarations */
static int detect_cpu_architecture(void);
static int detect_memory_capacity(void);
static int detect_cpu_features(void);
static int detect_gpu_info(void);
static int detect_storage_devices(void);
static int detect_network_devices(void);
static device_class_t classify_device(const hal_capability_t *cap);
static int register_builtin_drivers(void);

/* Initialize HAL */
hal_init_t hal_init(void) {
    if (hal_state.initialized) {
        return HAL_SUCCESS; /* Already initialized */
    }
    
    printk("Initializing Hardware Abstraction Layer...\n");
    
    /* Initialize HAL state */
    memset(&hal_state, 0, sizeof(hal_state));
    INIT_LIST_HEAD(&hal_state.driver_list);
    INIT_LIST_HEAD(&hal_state.device_list);
    
    /* Detect hardware capabilities */
    int ret = detect_cpu_architecture();
    if (ret != HAL_SUCCESS) {
        printk("Failed to detect CPU architecture\n");
        return ret;
    }
    
    ret = detect_memory_capacity();
    if (ret != HAL_SUCCESS) {
        printk("Failed to detect memory capacity\n");
        return ret;
    }
    
    ret = detect_cpu_features();
    if (ret != HAL_SUCCESS) {
        printk("Failed to detect CPU features\n");
        return ret;
    }
    
    ret = detect_gpu_info();
    if (ret != HAL_SUCCESS) {
        printk("Failed to detect GPU info (continuing without GPU)\n");
        /* Non-critical error, continue */
    }
    
    ret = detect_storage_devices();
    if (ret != HAL_SUCCESS) {
        printk("Failed to detect storage devices (continuing)\n");
        /* Non-critical error, continue */
    }
    
    ret = detect_network_devices();
    if (ret != HAL_SUCCESS) {
        printk("Failed to detect network devices (continuing)\n");
        /* Non-critical error, continue */
    }
    
    /* Classify device based on detected capabilities */
    hal_state.capabilities.device_class = classify_device(&hal_state.capabilities);
    
    /* Register built-in drivers */
    ret = register_builtin_drivers();
    if (ret != HAL_SUCCESS) {
        printk("Failed to register built-in drivers\n");
        return ret;
    }
    
    hal_state.initialized = true;
    
    printk("Hardware Abstraction Layer initialized\n");
    printk("  Architecture: %s\n", hal_state.capabilities.architecture_name);
    printk("  Memory: %lu MB\n", hal_state.capabilities.memory_capacity / (1024 * 1024));
    printk("  CPU Cores: %u\n", hal_state.capabilities.cpu_cores);
    printk("  Device Class: %s\n", device_class_name(hal_state.capabilities.device_class));
    
    return HAL_SUCCESS;
}

/* Get hardware capabilities */
hal_capability_t* hal_get_capabilities(void) {
    if (!hal_state.initialized) {
        return NULL;
    }
    
    return &hal_state.capabilities;
}

/* Profile all devices */
int hal_profile_devices(hal_capability_t *cap) {
    if (!hal_state.initialized || !cap) {
        return HAL_ERROR;
    }
    
    printk("Profiling hardware devices...\n");
    
    /* This would profile all detected devices */
    /* For now, just return success */
    
    return HAL_SUCCESS;
}

/* Register a driver */
int hal_register_driver(hal_driver_t *driver) {
    if (!hal_state.initialized || !driver || !driver->name) {
        return HAL_ERROR;
    }
    
    /* Check if driver already registered */
    hal_driver_t *existing = hal_find_driver(driver->name);
    if (existing) {
        printk("Driver %s already registered\n", driver->name);
        return HAL_ERROR;
    }
    
    /* Add to driver list */
    list_add(&driver->list, &hal_state.driver_list);
    hal_state.num_drivers++;
    
    printk("Registered driver: %s\n", driver->name);
    
    return HAL_SUCCESS;
}

/* Unregister a driver */
int hal_unregister_driver(const char *name) {
    if (!hal_state.initialized || !name) {
        return HAL_ERROR;
    }
    
    hal_driver_t *driver = hal_find_driver(name);
    if (!driver) {
        printk("Driver %s not found\n", name);
        return HAL_NOT_FOUND;
    }
    
    /* Remove from list */
    list_del(&driver->list);
    hal_state.num_drivers--;
    
    printk("Unregistered driver: %s\n", name);
    
    return HAL_SUCCESS;
}

/* Find a driver by name */
hal_driver_t* hal_find_driver(const char *name) {
    if (!hal_state.initialized || !name) {
        return NULL;
    }
    
    hal_driver_t *driver;
    list_for_each(driver, &hal_state.driver_list) {
        if (strcmp(driver->name, name) == 0) {
            return driver;
        }
    }
    
    return NULL;
}

/* Register a device */
int hal_register_device(hal_device_t *device) {
    if (!hal_state.initialized || !device || !device->name) {
        return HAL_ERROR;
    }
    
    /* Add to device list */
    list_add(&device->list, &hal_state.device_list);
    hal_state.num_devices++;
    
    printk("Registered device: %s (type: %u)\n", device->name, device->type);
    
    return HAL_SUCCESS;
}

/* Unregister a device */
int hal_unregister_device(const char *name) {
    if (!hal_state.initialized || !name) {
        return HAL_ERROR;
    }
    
    hal_device_t *device = hal_find_device(name);
    if (!device) {
        printk("Device %s not found\n", name);
        return HAL_NOT_FOUND;
    }
    
    /* Remove from list */
    list_del(&device->list);
    hal_state.num_devices--;
    
    printk("Unregistered device: %s\n", name);
    
    return HAL_SUCCESS;
}

/* Find a device by name */
hal_device_t* hal_find_device(const char *name) {
    if (!hal_state.initialized || !name) {
        return NULL;
    }
    
    hal_device_t *device;
    list_for_each(device, &hal_state.device_list) {
        if (strcmp(device->name, name) == 0) {
            return device;
        }
    }
    
    return NULL;
}

/* Print HAL information */
void hal_print_info(void) {
    if (!hal_state.initialized) {
        printk("HAL not initialized\n");
        return;
    }
    
    printk("=== Hardware Abstraction Layer Info ===\n");
    printk("Initialized: %s\n", hal_state.initialized ? "Yes" : "No");
    printk("Drivers registered: %u\n", hal_state.num_drivers);
    printk("Devices registered: %u\n", hal_state.num_devices);
    printk("Architecture: %s\n", hal_state.capabilities.architecture_name);
    printk("CPU Cores: %u\n", hal_state.capabilities.cpu_cores);
    printk("Memory: %lu MB\n", hal_state.capabilities.memory_capacity / (1024 * 1024));
    printk("Device Class: %s\n", device_class_name(hal_state.capabilities.device_class));
    printk("=========================================\n");
}

/* Power management functions */
int hal_set_power_state(hal_power_state_t state) {
    if (!hal_state.initialized) {
        return HAL_ERROR;
    }
    
    printk("Setting power state to %d\n", state);
    
    /* TODO: Implement actual power state management */
    
    return HAL_SUCCESS;
}

int hal_get_power_info(hal_power_info_t *info) {
    if (!hal_state.initialized || !info) {
        return HAL_ERROR;
    }
    
    /* Fill in power information */
    memset(info, 0, sizeof(hal_power_info_t));
    info->current_state = POWER_ON;
    info->battery_level = 100; /* Assume full battery for testing */
    info->ac_connected = true; /* Assume AC connected */
    info->estimated_remaining_time = 3600; /* 1 hour */
    
    return HAL_SUCCESS;
}

/* Private helper functions */

/* Detect CPU architecture */
static int detect_cpu_architecture(void) {
#if defined(__x86_64__) || defined(__x86_64)
    hal_state.capabilities.architecture = ARCH_X86_64;
    strcpy(hal_state.capabilities.architecture_name, "x86_64");
#elif defined(__aarch64__) || defined(__arm64__)
    hal_state.capabilities.architecture = ARCH_ARM64;
    strcpy(hal_state.capabilities.architecture_name, "ARM64");
#elif defined(__riscv) && __riscv_xlen == 64
    hal_state.capabilities.architecture = ARCH_RISCV;
    strcpy(hal_state.capabilities.architecture_name, "RISC-V");
#elif defined(__powerpc64__) || defined(__ppc64__)
    hal_state.capabilities.architecture = ARCH_POWERPC;
    strcpy(hal_state.capabilities.architecture_name, "PowerPC");
#elif defined(__mips__) && defined(__mips64)
    hal_state.capabilities.architecture = ARCH_MIPS;
    strcpy(hal_state.capabilities.architecture_name, "MIPS");
#else
    printk("Unknown architecture detected\n");
    return HAL_ERROR;
#endif
    
    printk("Detected architecture: %s\n", hal_state.capabilities.architecture_name);
    return HAL_SUCCESS;
}

/* Detect memory capacity */
static int detect_memory_capacity(void) {
    /* For now, use a reasonable default */
    /* In a real implementation, this would query actual memory */
    
    #if defined(__x86_64__) || defined(__x86_64)
        /* x86_64: Assume 8GB for testing */
        hal_state.capabilities.memory_capacity = 8ULL * 1024 * 1024 * 1024;
    #elif defined(__aarch64__) || defined(__arm64__)
        /* ARM64: Assume 4GB for testing */
        hal_state.capabilities.memory_capacity = 4ULL * 1024 * 1024 * 1024;
    #else
        /* Other architectures: Assume 2GB for testing */
        hal_state.capabilities.memory_capacity = 2ULL * 1024 * 1024 * 1024;
    #endif
    
    printk("Detected memory capacity: %lu MB\n", 
           hal_state.capabilities.memory_capacity / (1024 * 1024));
    
    return HAL_SUCCESS;
}

/* Detect CPU features */
static int detect_cpu_features(void) {
#if defined(__x86_64__) || defined(__x86_64)
    /* x86_64 CPUID detection */
    uint32_t cpuinfo[4];
    
    /* Get max CPUID level */
    __asm__ __volatile__("cpuid" : "=a"(cpuinfo[0]), "=b"(cpuinfo[1]), 
                        "=c"(cpuinfo[2]), "=d"(cpuinfo[3]) : "a"(0));
    
    if (cpuinfo[0] >= 1) {
        /* Get CPUID level 1 (processor info and features) */
        __asm__ __volatile__("cpuid" : "=a"(cpuinfo[0]), "=b"(cpuinfo[1]), 
                            "=c"(cpuinfo[2]), "=d"(cpuinfo[3]) : "a"(1));
        
        /* Store feature bits */
        hal_state.capabilities.cpu_features_edx = cpuinfo[3];
        hal_state.capabilities.cpu_features_ecx = cpuinfo[2];
        
        /* Parse individual features */
        hal_state.capabilities.cpu_features_sse = (cpuinfo[3] & (1 << 25)) != 0;
        hal_state.capabilities.cpu_features_sse2 = (cpuinfo[3] & (1 << 26)) != 0;
        hal_state.capabilities.cpu_features_avx = (cpuinfo[2] & (1 << 28)) != 0;
        hal_state.capabilities.cpu_features_avx2 = (cpuinfo[2] & (1 << 28)) && (cpuinfo[0] >= 7);
        hal_state.capabilities.cpu_features_bmi1 = (cpuinfo[2] & (1 << 3)) != 0;
        hal_state.capabilities.cpu_features_bmi2 = (cpuinfo[2] & (1 << 8)) != 0;
        hal_state.capabilities.cpu_features_popcnt = (cpuinfo[2] & (1 << 23)) != 0;
    }
    
    /* Get logical CPU count */
    __asm__ __volatile__("cpuid" : "=a"(cpuinfo[0]), "=b"(cpuinfo[1]), 
                        "=c"(cpuinfo[2]), "=d"(cpuinfo[3]) : "a"(1));
    
    hal_state.capabilities.cpu_logical_cores = (cpuinfo[1] >> 16) & 0xFF;
    hal_state.capabilities.cpu_cores = hal_state.capabilities.cpu_logical_cores;
    
    /* Get CPU model name */
    strcpy(hal_state.capabilities.cpu_model_name, "x86_64 CPU");
    
    printk("CPU features: SSE=%s, SSE2=%s, AVX=%s, BMI1=%s, BMI2=%s\n",
           hal_state.capabilities.cpu_features_sse ? "Yes" : "No",
           hal_state.capabilities.cpu_features_sse2 ? "Yes" : "No",
           hal_state.capabilities.cpu_features_avx ? "Yes" : "No",
           hal_state.capabilities.cpu_features_bmi1 ? "Yes" : "No",
           hal_state.capabilities.cpu_features_bmi2 ? "Yes" : "No");
    
#else
    /* Non-x86 architectures: set default values */
    hal_state.capabilities.cpu_features_sse = false;
    hal_state.capabilities.cpu_features_sse2 = false;
    hal_state.capabilities.cpu_features_avx = false;
    hal_state.capabilities.cpu_features_avx2 = false;
    hal_state.capabilities.cpu_features_bmi1 = false;
    hal_state.capabilities.cpu_features_bmi2 = false;
    hal_state.capabilities.cpu_features_popcnt = false;
    hal_state.capabilities.cpu_cores = 4; /* Default */
    hal_state.capabilities.cpu_logical_cores = 4;
    
    strcpy(hal_state.capabilities.cpu_model_name, "Non-x86 CPU");
    
    printk("Non-x86 CPU detected, using default values\n");
#endif
    
    return HAL_SUCCESS;
}

/* Detect GPU information */
static int detect_gpu_info(void) {
    /* For now, assume no GPU or basic GPU */
    /* In a real implementation, this would probe GPU hardware */
    
    hal_state.capabilities.has_gpu = false;
    hal_state.capabilities.gpu_vendor = GPU_VENDOR_NONE;
    hal_state.capabilities.gpu_memory = 0;
    
    /* Try to detect NVIDIA GPU */
    /* This would involve PCI enumeration or other detection methods */
    
    printk("GPU detection: %s\n", hal_state.capabilities.has_gpu ? "GPU found" : "No GPU detected");
    
    return HAL_SUCCESS;
}

/* Detect storage devices */
static int detect_storage_devices(void) {
    /* For now, assume basic storage */
    /* In a real implementation, this would probe storage controllers */
    
    hal_state.capabilities.storage_devices = 1; /* Assume one storage device */
    hal_state.capabilities.storage_types = 0x01; /* Assume basic storage type */
    
    printk("Storage detection: %u devices detected\n", hal_state.capabilities.storage_devices);
    
    return HAL_SUCCESS;
}

/* Detect network devices */
static int detect_network_devices(void) {
    /* For now, assume no network devices */
    /* In a real implementation, this would probe network controllers */
    
    hal_state.capabilities.network_interfaces = 0;
    
    printk("Network detection: %u interfaces detected\n", hal_state.capabilities.network_interfaces);
    
    return HAL_SUCCESS;
}

/* Classify device based on capabilities */
static device_class_t classify_device(const hal_capability_t *cap) {
    uint64_t memory_mb = cap->memory_capacity / (1024 * 1024);
    
    if (memory_mb < 1024) { /* < 1GB */
        return DEVICE_IOT;
    } else if (memory_mb < 4096) { /* 1-4GB */
        return DEVICE_EMBEDDED;
    } else if (memory_mb < 32768) { /* 4-32GB */
        return DEVICE_DESKTOP;
    } else if (memory_mb < 262144) { /* 32GB-256GB */
        return DEVICE_SERVER;
    } else if (memory_mb < 1048576) { /* 256GB-1TB */
        return DEVICE_WORKSTATION;
    } else { /* 1TB+ */
        return DEVICE_SUPERCOMPUTER;
    }
}

/* Register built-in drivers */
static int register_builtin_drivers(void) {
    printk("Registering built-in drivers...\n");
    
    /* CPU driver */
    hal_driver_t *cpu_driver = (hal_driver_t *)kmalloc(sizeof(hal_driver_t), GFP_KERNEL);
    if (cpu_driver) {
        cpu_driver->name = "cpu_driver";
        cpu_driver->description = "CPU architecture driver";
        cpu_driver->type = 0x0001; /* CPU driver type */
        cpu_driver->init = NULL;
        cpu_driver->probe = NULL;
        cpu_driver->shutdown = NULL;
        hal_register_driver(cpu_driver);
    }
    
    /* Memory driver */
    hal_driver_t *mem_driver = (hal_driver_t *)kmalloc(sizeof(hal_driver_t), GFP_KERNEL);
    if (mem_driver) {
        mem_driver->name = "memory_driver";
        mem_driver->description = "Memory management driver";
        mem_driver->type = 0x0002; /* Memory driver type */
        mem_driver->init = NULL;
        mem_driver->probe = NULL;
        mem_driver->shutdown = NULL;
        hal_register_driver(mem_driver);
    }
    
    /* Timer driver */
    hal_driver_t *timer_driver = (hal_driver_t *)kmalloc(sizeof(hal_driver_t), GFP_KERNEL);
    if (timer_driver) {
        timer_driver->name = "timer_driver";
        timer_driver->description = "Timer driver";
        timer_driver->type = 0x0004; /* Timer driver type */
        timer_driver->init = NULL;
        timer_driver->probe = NULL;
        timer_driver->shutdown = NULL;
        hal_register_driver(timer_driver);
    }
    
    /* Serial driver */
    hal_driver_t *serial_driver = (hal_driver_t *)kmalloc(sizeof(hal_driver_t), GFP_KERNEL);
    if (serial_driver) {
        serial_driver->name = "serial_driver";
        serial_driver->description = "Serial port driver";
        serial_driver->type = 0x0008; /* Serial driver type */
        serial_driver->init = NULL;
        serial_driver->probe = NULL;
        serial_driver->shutdown = NULL;
        hal_register_driver(serial_driver);
    }
    
    printk("Built-in drivers registered: %u\n", hal_state.num_drivers);
    
    return HAL_SUCCESS;
}

/* Device class name lookup */
const char* device_class_name(device_class_t device_class) {
    static const char* device_class_names[] = {
        "IoT",
        "Embedded", 
        "Desktop",
        "Server",
        "Workstation",
        "Supercomputer"
    };
    
    if (device_class < DEVICE_SUPERCOMPUTER + 1) {
        return device_class_names[device_class];
    }
    
    return "Unknown";
}