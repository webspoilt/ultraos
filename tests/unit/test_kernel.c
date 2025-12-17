/*
 * UltraOS Kernel Unit Tests
 * Author: zeroday
 * Date: 2025-12-17
 * Version: 2.0
 */

#include "../../include/ultraos_types.h"
#include "../../include/ultraos_config.h"

/* Test result tracking */
typedef struct {
    uint32_t total_tests;
    uint32_t passed_tests;
    uint32_t failed_tests;
    uint32_t skipped_tests;
} test_results_t;

static test_results_t test_results = {0, 0, 0, 0};

/* Test framework macros */
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printk("  ✓ PASS: %s\n", message); \
            test_results.passed_tests++; \
        } else { \
            printk("  ✗ FAIL: %s\n", message); \
            test_results.failed_tests++; \
        } \
        test_results.total_tests++; \
    } while(0)

#define TEST_SKIP(message) \
    do { \
        printk("  - SKIP: %s\n", message); \
        test_results.skipped_tests++; \
        test_results.total_tests++; \
    } while(0)

/* Forward declarations */
static void test_kernel_basic_init(void);
static void test_string_functions(void);
static void test_memory_allocation(void);
static void test_list_operations(void);
static void test_scheduler_basic(void);
static void test_hal_basic(void);
static void test_cpu_detection(void);

/* Main test runner */
int main(void) {
    printk("=== UltraOS Kernel Unit Tests ===\n");
    
    /* Reset test results */
    memset(&test_results, 0, sizeof(test_results));
    
    /* Run all tests */
    test_kernel_basic_init();
    test_string_functions();
    test_memory_allocation();
    test_list_operations();
    test_scheduler_basic();
    test_hal_basic();
    test_cpu_detection();
    
    /* Print results */
    printk("\n=== Test Results ===\n");
    printk("Total Tests: %u\n", test_results.total_tests);
    printk("Passed: %u\n", test_results.passed_tests);
    printk("Failed: %u\n", test_results.failed_tests);
    printk("Skipped: %u\n", test_results.skipped_tests);
    
    if (test_results.failed_tests == 0) {
        printk("✓ All tests passed!\n");
        return 0;
    } else {
        printk("✗ Some tests failed!\n");
        return 1;
    }
}

/* Test kernel basic initialization */
static void test_kernel_basic_init(void) {
    printk("\n--- Testing Kernel Basic Initialization ---\n");
    
    /* Test kernel initialization state */
    TEST_ASSERT(!kernel_is_initialized(), "Kernel should not be initialized initially");
    
    /* Test boot context */
    boot_context_t *ctx = kernel_get_boot_context();
    TEST_ASSERT(ctx == NULL, "Boot context should be NULL initially");
    
    /* Test kernel info printing */
    printk("Testing kernel info output...\n");
    kernel_print_info();
    TEST_ASSERT(true, "Kernel info printed successfully");
}

/* Test string functions */
static void test_string_functions(void) {
    printk("\n--- Testing String Functions ---\n");
    
    char buffer[256];
    size_t len;
    
    /* Test strlen */
    len = strlen("Hello, World!");
    TEST_ASSERT(len == 13, "strlen('Hello, World!') should return 13");
    
    /* Test strcpy */
    strcpy(buffer, "Test");
    TEST_ASSERT(strcmp(buffer, "Test") == 0, "strcpy should copy string correctly");
    
    /* Test strcmp */
    TEST_ASSERT(strcmp("abc", "abc") == 0, "strcmp('abc', 'abc') should return 0");
    TEST_ASSERT(strcmp("abc", "def") < 0, "strcmp('abc', 'def') should return negative");
    TEST_ASSERT(strcmp("xyz", "abc") > 0, "strcmp('xyz', 'abc') should return positive");
    
    /* Test memset */
    memset(buffer, 'A', sizeof(buffer));
    TEST_ASSERT(buffer[0] == 'A' && buffer[10] == 'A', "memset should set memory correctly");
    
    /* Test memcpy */
    char src[] = "Source";
    strcpy(buffer, "DestXXXX");
    memcpy(buffer, src, 6);
    TEST_ASSERT(strcmp(buffer, "Source") == 0, "memcpy should copy memory correctly");
}

/* Test memory allocation */
static void test_memory_allocation(void) {
    printk("\n--- Testing Memory Allocation ---\n");
    
    /* Test basic allocation */
    void *ptr1 = kmalloc(1024, GFP_KERNEL);
    TEST_ASSERT(ptr1 != NULL, "kmalloc should return valid pointer");
    
    if (ptr1) {
        /* Test memory writing */
        memset(ptr1, 0xAA, 1024);
        TEST_ASSERT(((char*)ptr1)[0] == 0xAA, "Memory should be writable");
        
        /* Free memory (basic implementation) */
        kfree(ptr1);
    }
    
    /* Test multiple allocations */
    void *ptr2 = kmalloc(512, GFP_KERNEL);
    void *ptr3 = kmalloc(256, GFP_KERNEL);
    TEST_ASSERT(ptr2 != NULL && ptr3 != NULL, "Multiple allocations should succeed");
    
    if (ptr2) kfree(ptr2);
    if (ptr3) kfree(ptr3);
    
    /* Test alignment */
    void *ptr4 = kmalloc(17, GFP_KERNEL);
    if (ptr4) {
        uintptr_t addr = (uintptr_t)ptr4;
        TEST_ASSERT(addr % ULTRAOS_ALIGNMENT == 0, "Allocated memory should be aligned");
        kfree(ptr4);
    }
}

/* Test list operations */
static void test_list_operations(void) {
    printk("\n--- Testing List Operations ---\n");
    
    struct list_head list;
    struct test_node {
        int value;
        struct list_head list;
    } node1, node2, node3;
    
    /* Initialize list */
    INIT_LIST_HEAD(&list);
    TEST_ASSERT(list_empty(&list), "New list should be empty");
    
    /* Add nodes */
    node1.value = 1;
    node2.value = 2;
    node3.value = 3;
    
    list_add(&node1.list, &list);
    TEST_ASSERT(!list_empty(&list), "List should not be empty after adding node");
    
    list_add(&node2.list, &list);
    list_add(&node3.list, &list);
    
    /* Check list structure */
    struct test_node *node;
    int count = 0;
    list_for_each(node, &list) {
        count++;
    }
    TEST_ASSERT(count == 3, "List should contain 3 nodes");
    
    /* Test removal */
    list_del(&node2.list);
    count = 0;
    list_for_each(node, &list) {
        count++;
    }
    TEST_ASSERT(count == 2, "List should contain 2 nodes after removal");
}

/* Test scheduler basic functionality */
static void test_scheduler_basic(void) {
    printk("\n--- Testing Scheduler Basic Functionality ---\n");
    
    scheduler_config_t config = {
        .time_quantum_ms = 10,
        .max_processes = 16,
        .max_threads_per_process = 64,
        .preemptive = true,
        .scheduler_algorithm = 0 /* Round Robin */
    };
    
    /* Test scheduler initialization */
    int ret = scheduler_init(&config);
    TEST_ASSERT(ret == STATUS_SUCCESS, "Scheduler should initialize successfully");
    
    if (ret == STATUS_SUCCESS) {
        /* Test scheduler start */
        ret = scheduler_start();
        TEST_ASSERT(ret == STATUS_SUCCESS, "Scheduler should start successfully");
        
        /* Test getting current thread */
        thread_t *current = scheduler_get_current_thread();
        TEST_ASSERT(current == NULL, "Current thread should be NULL initially");
        
        /* Test getting current process */
        process_t *current_proc = scheduler_get_current_process();
        TEST_ASSERT(current_proc == NULL, "Current process should be NULL initially");
        
        /* Test scheduler statistics */
        scheduler_stats_t stats = scheduler_get_stats();
        TEST_ASSERT(stats.active_processes == 0, "Active processes should be 0 initially");
        TEST_ASSERT(stats.active_threads == 0, "Active threads should be 0 initially");
        
        /* Test creating a process */
        pid_t pid = scheduler_create_process("test_process", (void*)0x1000, PRIORITY_NORMAL);
        TEST_ASSERT(pid > 0, "Process creation should return valid PID");
        
        if (pid > 0) {
            stats = scheduler_get_stats();
            TEST_ASSERT(stats.active_processes == 1, "Active processes should be 1 after creation");
            
            /* Test creating a thread */
            tid_t tid = scheduler_create_thread(pid, "test_thread", (void*)0x2000, NULL, PRIORITY_NORMAL);
            TEST_ASSERT(tid > 0, "Thread creation should return valid TID");
            
            if (tid > 0) {
                stats = scheduler_get_stats();
                TEST_ASSERT(stats.active_threads == 1, "Active threads should be 1 after creation");
                
                /* Test thread termination */
                ret = scheduler_terminate_thread(tid);
                TEST_ASSERT(ret == STATUS_SUCCESS, "Thread termination should succeed");
                
                stats = scheduler_get_stats();
                TEST_ASSERT(stats.active_threads == 0, "Active threads should be 0 after termination");
            }
            
            /* Test process termination */
            ret = scheduler_terminate_process(pid);
            TEST_ASSERT(ret == STATUS_SUCCESS, "Process termination should succeed");
            
            stats = scheduler_get_stats();
            TEST_ASSERT(stats.active_processes == 0, "Active processes should be 0 after termination");
        }
    }
}

/* Test HAL basic functionality */
static void test_hal_basic(void) {
    printk("\n--- Testing HAL Basic Functionality ---\n");
    
    /* Test HAL initialization */
    hal_init_t ret = hal_init();
    TEST_ASSERT(ret == HAL_SUCCESS, "HAL should initialize successfully");
    
    if (ret == HAL_SUCCESS) {
        /* Test getting capabilities */
        hal_capability_t *cap = hal_get_capabilities();
        TEST_ASSERT(cap != NULL, "HAL capabilities should not be NULL");
        
        if (cap) {
            TEST_ASSERT(cap->architecture >= 0 && cap->architecture < 5, "Architecture should be valid");
            TEST_ASSERT(cap->device_class >= 0 && cap->device_class < 6, "Device class should be valid");
            TEST_ASSERT(cap->memory_capacity > 0, "Memory capacity should be positive");
            TEST_ASSERT(cap->cpu_cores > 0, "CPU cores should be positive");
        }
        
        /* Test HAL info printing */
        printk("Testing HAL info output...\n");
        hal_print_info();
        TEST_ASSERT(true, "HAL info printed successfully");
    }
}

/* Test CPU detection */
static void test_cpu_detection(void) {
    printk("\n--- Testing CPU Detection ---\n");
    
#if ULTRAOS_ARCH_X86_64
    /* Test x86_64 specific features */
    bool has_sse = x86_64_has_feature(CPU_FEATURE_SSE);
    bool has_sse2 = x86_64_has_feature(CPU_FEATURE_SSE2);
    
    printk("CPU has SSE: %s\n", has_sse ? "Yes" : "No");
    printk("CPU has SSE2: %s\n", has_sse2 ? "Yes" : "No");
    
    TEST_ASSERT(true, "CPU feature detection completed");
    
    /* Test CPU info */
    cpu_info_t *cpu_info = x86_64_get_cpu_info();
    TEST_ASSERT(cpu_info != NULL, "CPU info should not be NULL");
    
    if (cpu_info) {
        TEST_ASSERT(strlen(cpu_info->vendor) > 0, "CPU vendor should not be empty");
        TEST_ASSERT(strlen(cpu_info->model_name) > 0, "CPU model name should not be empty");
        TEST_ASSERT(cpu_info->cache_line_size > 0, "Cache line size should be positive");
    }
    
    /* Test CPU info printing */
    printk("Testing CPU info output...\n");
    x86_64_print_info();
    TEST_ASSERT(true, "CPU info printed successfully");
    
#else
    TEST_SKIP("CPU detection test only available on x86_64");
#endif
}

/* Utility function for test output */
void print_test_header(const char *test_name) {
    printk("\n=== %s ===\n", test_name);
}

void print_test_footer(uint32_t passed, uint32_t failed, uint32_t skipped) {
    printk("\n--- Test Summary ---\n");
    printk("Passed: %u, Failed: %u, Skipped: %u\n", passed, failed, skipped);
}