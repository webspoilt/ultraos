/*
 * UltraOS x86_64 CPU Driver
 * Author: zeroday
 * Date: 2025-12-17
 * Version: 2.0
 */

#include "../../../include/ultraos_types.h"
#include "../../../include/ultraos_config.h"

/* CPU information */
static cpu_info_t cpu_info;
static bool cpu_initialized = false;

/* CPUID instruction wrapper */
static void cpuid(uint32_t leaf, uint32_t subleaf, 
                  uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ __volatile__(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(subleaf)
    );
}

/* Initialize x86_64 CPU */
int x86_64_cpu_init(void) {
    if (cpu_initialized) {
        return STATUS_ALREADY_INITIALIZED;
    }
    
    printk("Initializing x86_64 CPU...\n");
    
    /* Initialize CPU info structure */
    memset(&cpu_info, 0, sizeof(cpu_info));
    cpu_info.id = 0;
    cpu_info.arch = ARCH_X86_64;
    cpu_info.family = 0;
    cpu_info.model = 0;
    cpu_info.stepping = 0;
    
    /* Get CPU vendor string */
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    
    /* Store vendor string */
    memcpy(cpu_info.vendor, &ebx, 4);
    memcpy(cpu_info.vendor + 4, &edx, 4);
    memcpy(cpu_info.vendor + 8, &ecx, 4);
    cpu_info.vendor[12] = '\0';
    
    /* Get CPU brand string */
    cpuid(0x80000002, 0, &eax, &ebx, &ecx, &edx);
    memcpy(cpu_info.model_name, &eax, 4);
    memcpy(cpu_info.model_name + 4, &ebx, 4);
    memcpy(cpu_info.model_name + 8, &ecx, 4);
    memcpy(cpu_info.model_name + 12, &edx, 4);
    
    cpuid(0x80000003, 0, &eax, &ebx, &ecx, &edx);
    memcpy(cpu_info.model_name + 16, &eax, 4);
    memcpy(cpu_info.model_name + 20, &ebx, 4);
    memcpy(cpu_info.model_name + 24, &ecx, 4);
    memcpy(cpu_info.model_name + 28, &edx, 4);
    
    cpuid(0x80000004, 0, &eax, &ebx, &ecx, &edx);
    memcpy(cpu_info.model_name + 32, &eax, 4);
    memcpy(cpu_info.model_name + 36, &ebx, 4);
    memcpy(cpu_info.model_name + 40, &ecx, 4);
    memcpy(cpu_info.model_name + 44, &edx, 4);
    cpu_info.model_name[48] = '\0';
    
    /* Get CPU features */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    
    /* Parse family and model */
    cpu_info.family = (eax >> 8) & 0xF;
    cpu_info.model = ((eax >> 4) & 0xF) | ((eax >> 12) & 0xF0);
    cpu_info.stepping = eax & 0xF;
    
    /* Store feature bits */
    cpu_info.features[0] = edx; /* Standard features */
    cpu_info.features[1] = ecx; /* Extended features */
    
    /* Get logical CPU count */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    cpu_info.cache_line_size = (ebx >> 8) & 0xFF;
    cpu_info.cache_sizes[0] = ((ebx >> 16) & 0xFF) * 1024; /* L1 */
    cpu_info.cache_sizes[1] = ((ebx >> 24) & 0xFF) * 1024; /* L2 */
    
    /* Get cache sizes */
    cpuid(4, 0, &eax, &ebx, &ecx, &edx);
    if ((eax & 0x1F) != 0) {
        /* L3 cache */
        cpu_info.cache_sizes[2] = ((eax >> 22) + 1) * (ebx + 1) * (ecx + 1);
    } else {
        cpu_info.cache_sizes[2] = 0; /* No L3 cache */
    }
    
    /* Get CPU frequency */
    cpuid(0x16, 0, &eax, &ebx, &ecx, &edx);
    cpu_info.frequency_mhz = eax;
    
    cpu_initialized = true;
    
    printk("x86_64 CPU initialized\n");
    printk("  Vendor: %s\n", cpu_info.vendor);
    printk("  Model: %s\n", cpu_info.model_name);
    printk("  Family: %u, Model: %u, Stepping: %u\n", 
           cpu_info.family, cpu_info.model, cpu_info.stepping);
    printk("  Cache: L1=%u KB, L2=%u KB, L3=%u KB\n",
           cpu_info.cache_sizes[0] / 1024,
           cpu_info.cache_sizes[1] / 1024,
           cpu_info.cache_sizes[2] / 1024);
    printk("  Frequency: %u MHz\n", cpu_info.frequency_mhz);
    
    return STATUS_SUCCESS;
}

/* Get CPU information */
cpu_info_t* x86_64_get_cpu_info(void) {
    if (!cpu_initialized) {
        return NULL;
    }
    
    return &cpu_info;
}

/* Get CPU features */
void x86_64_get_features(bool *features) {
    if (!cpu_initialized || !features) {
        return;
    }
    
    /* Initialize all features to false */
    memset(features, 0, sizeof(bool) * CPU_FEATURE_MAX);
    
    /* Parse feature bits from CPUID */
    uint32_t edx = cpu_info.features[0];
    uint32_t ecx = cpu_info.features[1];
    
    /* Standard features (EDX) */
    features[CPU_FEATURE_FPU] = (edx & (1 << 0)) != 0;
    features[CPU_FEATURE_VME] = (edx & (1 << 1)) != 0;
    features[CPU_FEATURE_DE] = (edx & (1 << 2)) != 0;
    features[CPU_FEATURE_PSE] = (edx & (1 << 3)) != 0;
    features[CPU_FEATURE_TSC] = (edx & (1 << 4)) != 0;
    features[CPU_FEATURE_MSR] = (edx & (1 << 5)) != 0;
    features[CPU_FEATURE_PAE] = (edx & (1 << 6)) != 0;
    features[CPU_FEATURE_MCE] = (edx & (1 << 7)) != 0;
    features[CPU_FEATURE_CX8] = (edx & (1 << 8)) != 0;
    features[CPU_FEATURE_APIC] = (edx & (1 << 9)) != 0;
    features[CPU_FEATURE_SEP] = (edx & (1 << 11)) != 0;
    features[CPU_FEATURE_MTRR] = (edx & (1 << 12)) != 0;
    features[CPU_FEATURE_PGE] = (edx & (1 << 13)) != 0;
    features[CPU_FEATURE_MCA] = (edx & (1 << 14)) != 0;
    features[CPU_FEATURE_CMOV] = (edx & (1 << 15)) != 0;
    features[CPU_FEATURE_PAT] = (edx & (1 << 16)) != 0;
    features[CPU_FEATURE_PSE36] = (edx & (1 << 17)) != 0;
    features[CPU_FEATURE_PN] = (edx & (1 << 18)) != 0;
    features[CPU_FEATURE_CLFLUSH] = (edx & (1 << 19)) != 0;
    features[CPU_FEATURE_MMX] = (edx & (1 << 23)) != 0;
    features[CPU_FEATURE_FXSR] = (edx & (1 << 24)) != 0;
    features[CPU_FEATURE_SSE] = (edx & (1 << 25)) != 0;
    features[CPU_FEATURE_SSE2] = (edx & (1 << 26)) != 0;
    features[CPU_FEATURE_SS] = (edx & (1 << 27)) != 0;
    features[CPU_FEATURE_HTT] = (edx & (1 << 28)) != 0;
    features[CPU_FEATURE_TM] = (edx & (1 << 29)) != 0;
    features[CPU_FEATURE_PBE] = (edx & (1 << 31)) != 0;
    
    /* Extended features (ECX) */
    features[CPU_FEATURE_SSE3] = (ecx & (1 << 0)) != 0;
    features[CPU_FEATURE_PCLMULQDQ] = (ecx & (1 << 1)) != 0;
    features[CPU_FEATURE_DTES64] = (ecx & (1 << 2)) != 0;
    features[CPU_FEATURE_MONITOR] = (ecx & (1 << 3)) != 0;
    features[CPU_FEATURE_DS_CPL] = (ecx & (1 << 4)) != 0;
    features[CPU_FEATURE_VMX] = (ecx & (1 << 5)) != 0;
    features[CPU_FEATURE_SMX] = (ecx & (1 << 6)) != 0;
    features[CPU_FEATURE_EST] = (ecx & (1 << 7)) != 0;
    features[CPU_FEATURE_TM2] = (ecx & (1 << 8)) != 0;
    features[CPU_FEATURE_SSSE3] = (ecx & (1 << 9)) != 0;
    features[CPU_FEATURE_CID] = (ecx & (1 << 10)) != 0;
    features[CPU_FEATURE_FMA] = (ecx & (1 << 12)) != 0;
    features[CPU_FEATURE_CX16] = (ecx & (1 << 13)) != 0;
    features[CPU_FEATURE_XTPR] = (ecx & (1 << 14)) != 0;
    features[CPU_FEATURE_PDCM] = (ecx & (1 << 15)) != 0;
    features[CPU_FEATURE_PCIDE] = (ecx & (1 << 17)) != 0;
    features[CPU_FEATURE_DCA] = (ecx & (1 << 18)) != 0;
    features[CPU_FEATURE_SSE4_1] = (ecx & (1 << 19)) != 0;
    features[CPU_FEATURE_SSE4_2] = (ecx & (1 << 20)) != 0;
    features[CPU_FEATURE_X2APIC] = (ecx & (1 << 21)) != 0;
    features[CPU_FEATURE_MOVBE] = (ecx & (1 << 22)) != 0;
    features[CPU_FEATURE_POPCNT] = (ecx & (1 << 23)) != 0;
    features[CPU_FEATURE_TSC_DEADLINE] = (ecx & (1 << 24)) != 0;
    features[CPU_FEATURE_AES] = (ecx & (1 << 25)) != 0;
    features[CPU_FEATURE_XSAVE] = (ecx & (1 << 26)) != 0;
    features[CPU_FEATURE_OSXSAVE] = (ecx & (1 << 27)) != 0;
    features[CPU_FEATURE_AVX] = (ecx & (1 << 28)) != 0;
    features[CPU_FEATURE_F16C] = (ecx & (1 << 29)) != 0;
    features[CPU_FEATURE_RDRAND] = (ecx & (1 << 30)) != 0;
    features[CPU_FEATURE_HYPERVISOR] = (ecx & (1 << 31)) != 0;
}

/* Print CPU information */
void x86_64_print_info(void) {
    if (!cpu_initialized) {
        printk("x86_64 CPU not initialized\n");
        return;
    }
    
    printk("=== x86_64 CPU Information ===\n");
    printk("Vendor: %s\n", cpu_info.vendor);
    printk("Model: %s\n", cpu_info.model_name);
    printk("Family: %u, Model: %u, Stepping: %u\n", 
           cpu_info.family, cpu_info.model, cpu_info.stepping);
    printk("Cache Line Size: %u bytes\n", cpu_info.cache_line_size);
    printk("Cache Sizes: L1=%u KB, L2=%u KB, L3=%u KB\n",
           cpu_info.cache_sizes[0] / 1024,
           cpu_info.cache_sizes[1] / 1024,
           cpu_info.cache_sizes[2] / 1024);
    printk("Frequency: %u MHz\n", cpu_info.frequency_mhz);
    
    /* Print key features */
    bool features[CPU_FEATURE_MAX];
    x86_64_get_features(features);
    
    printk("Key Features:\n");
    if (features[CPU_FEATURE_SSE]) printk("  SSE\n");
    if (features[CPU_FEATURE_SSE2]) printk("  SSE2\n");
    if (features[CPU_FEATURE_SSE3]) printk("  SSE3\n");
    if (features[CPU_FEATURE_SSSE3]) printk("  SSSE3\n");
    if (features[CPU_FEATURE_SSE4_1]) printk("  SSE4.1\n");
    if (features[CPU_FEATURE_SSE4_2]) printk("  SSE4.2\n");
    if (features[CPU_FEATURE_AVX]) printk("  AVX\n");
    if (features[CPU_FEATURE_FMA]) printk("  FMA\n");
    if (features[CPU_FEATURE_BMI1]) printk("  BMI1\n");
    if (features[CPU_FEATURE_BMI2]) printk("  BMI2\n");
    if (features[CPU_FEATURE_POPCNT]) printk("  POPCNT\n");
    if (features[CPU_FEATURE_RDRAND]) printk("  RDRAND\n");
    if (features[CPU_FEATURE_AES]) printk("  AES-NI\n");
    
    printk("================================\n");
}

/* Check if CPU feature is supported */
bool x86_64_has_feature(cpu_feature_t feature) {
    if (!cpu_initialized) {
        return false;
    }
    
    bool features[CPU_FEATURE_MAX];
    x86_64_get_features(features);
    
    return features[feature];
}

/* Get CPU cache information */
void x86_64_get_cache_info(uint32_t *l1_size, uint32_t *l2_size, uint32_t *l3_size) {
    if (!cpu_initialized) {
        if (l1_size) *l1_size = 0;
        if (l2_size) *l2_size = 0;
        if (l3_size) *l3_size = 0;
        return;
    }
    
    if (l1_size) *l1_size = cpu_info.cache_sizes[0];
    if (l2_size) *l2_size = cpu_info.cache_sizes[1];
    if (l3_size) *l3_size = cpu_info.cache_sizes[2];
}

/* CPU performance monitoring */
uint64_t x86_64_read_tsc(void) {
    uint64_t tsc;
    __asm__ __volatile__("rdtsc" : "=A"(tsc));
    return tsc;
}

/* CPU frequency detection */
uint64_t x86_64_get_cpu_frequency_hz(void) {
    /* This is a simplified implementation */
    /* In practice, you'd use more sophisticated methods */
    return (uint64_t)cpu_info.frequency_mhz * 1000000ULL;
}

/* Register x86_64 driver with HAL */
int x86_register_driver(void) {
    return x86_64_cpu_init();
}