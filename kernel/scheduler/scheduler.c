/*
 * UltraOS Scheduler Implementation
 * Author: zeroday
 * Date: 2025-12-17
 * Version: 2.0
 */

#include "../../include/ultraos_types.h"
#include "../../include/ultraos_config.h"
#include "scheduler.h"

/* Scheduler state */
static struct {
    bool initialized;
    bool running;
    scheduler_config_t config;
    thread_t *ready_queue[ULTRAOS_MAX_PRIORITY_LEVELS];
    thread_t *current_thread;
    process_t *process_list;
    uint32_t next_tid;
    uint32_t next_pid;
    uint64_t system_time;
    scheduler_stats_t stats;
    thread_t *sleeping_threads;
} scheduler_state;

/* Forward declarations */
static thread_t* get_next_thread(void);
static void add_to_ready_queue(thread_t *thread);
static void remove_from_ready_queue(thread_t *thread);
static thread_t* scheduler_tick_round_robin(void);
static thread_t* scheduler_tick_priority(void);
static thread_t* scheduler_tick_cfs(void);
static thread_t* scheduler_tick_multiqueue(void);
static void scheduler_update_stats(void);
static process_t* find_process_by_pid(uint32_t pid);
static thread_t* find_thread_by_tid(uint32_t tid);
static void free_thread_resources(thread_t *thread);
static void free_process_resources(process_t *process);
static void process_table_init(void);
static void thread_table_init(void);

/* Initialize the scheduler */
int scheduler_init(const scheduler_config_t *config) {
    if (scheduler_state.initialized) {
        return STATUS_ALREADY_INITIALIZED;
    }
    
    if (!config) {
        return STATUS_INVALID_PARAMETER;
    }
    
    /* Initialize scheduler state */
    memset(&scheduler_state, 0, sizeof(scheduler_state));
    
    /* Copy configuration */
    scheduler_state.config = *config;
    scheduler_state.next_tid = 1;
    scheduler_state.next_pid = 1;
    scheduler_state.system_time = 0;
    
    /* Initialize priority queues */
    for (int i = 0; i < ULTRAOS_MAX_PRIORITY_LEVELS; i++) {
        scheduler_state.ready_queue[i] = NULL;
    }
    
    scheduler_state.process_list = NULL;
    scheduler_state.current_thread = NULL;
    scheduler_state.sleeping_threads = NULL;
    
    /* Initialize statistics */
    memset(&scheduler_state.stats, 0, sizeof(scheduler_stats_t));
    
    /* Initialize process and thread tables */
    process_table_init();
    thread_table_init();
    
    scheduler_state.initialized = true;
    printk("Scheduler initialized (algorithm: %u, quantum: %u ms)\n",
           config->scheduler_algorithm, config->time_quantum_ms);
    
    return STATUS_SUCCESS;
}

/* Start the scheduler */
int scheduler_start(void) {
    if (!scheduler_state.initialized) {
        return STATUS_NOT_INITIALIZED;
    }
    
    scheduler_state.running = true;
    printk("Scheduler started\n");
    
    return STATUS_SUCCESS;
}

/* Stop the scheduler */
void scheduler_stop(void) {
    scheduler_state.running = false;
    printk("Scheduler stopped\n");
}

/* Main scheduling function */
void scheduler_schedule(void) {
    if (!scheduler_state.running || !scheduler_state.current_thread) {
        return;
    }
    
    thread_t *old_thread = scheduler_state.current_thread;
    thread_t *new_thread = get_next_thread();
    
    if (!new_thread || new_thread == old_thread) {
        return;
    }
    
    /* Switch context */
    scheduler_switch_context(old_thread, new_thread);
    scheduler_state.current_thread = new_thread;
    
    /* Update statistics */
    scheduler_state.stats.total_context_switches++;
    scheduler_update_stats();
}

/* Yield CPU to next thread */
void scheduler_yield(void) {
    if (!scheduler_state.running) {
        return;
    }
    
    if (scheduler_state.current_thread) {
        scheduler_state.current_thread->state = STATE_READY;
        add_to_ready_queue(scheduler_state.current_thread);
        scheduler_schedule();
    }
}

/* Timer tick handler */
void scheduler_timer_tick(void) {
    if (!scheduler_state.running || !scheduler_state.current_thread) {
        return;
    }
    
    scheduler_state.system_time += scheduler_state.config.time_quantum_ms;
    scheduler_state.current_thread->time_slice_used += scheduler_state.config.time_quantum_ms;
    
    /* Check if time slice exhausted */
    if (scheduler_state.current_thread->time_slice_used >= scheduler_state.config.time_quantum_ms) {
        scheduler_state.current_thread->state = STATE_READY;
        scheduler_state.current_thread->time_slice_used = 0;
        add_to_ready_queue(scheduler_state.current_thread);
        scheduler_schedule();
    }
    
    /* Check for sleeping threads */
    thread_t *thread = scheduler_state.sleeping_threads;
    while (thread) {
        thread_t *next = thread->next;
        if (thread->state == STATE_SLEEPING && 
            scheduler_state.system_time >= thread->wake_time) {
            thread->state = STATE_READY;
            thread->wake_time = 0;
            
            /* Remove from sleeping list */
            if (thread->prev) {
                thread->prev->next = thread->next;
            } else {
                scheduler_state.sleeping_threads = thread->next;
            }
            if (thread->next) {
                thread->next->prev = thread->prev;
            }
            
            /* Add to ready queue */
            add_to_ready_queue(thread);
        }
        thread = next;
    }
}

/* Create a new process */
int scheduler_create_process(const char *name, void *entry_point, uint32_t priority) {
    if (!scheduler_state.initialized || !name || !entry_point) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (scheduler_state.stats.active_processes >= scheduler_state.config.max_processes) {
        return STATUS_OUT_OF_RESOURCES;
    }
    
    /* Allocate process control block */
    process_t *process = (process_t *)kmalloc(sizeof(process_t), GFP_KERNEL);
    if (!process) {
        return STATUS_OUT_OF_MEMORY;
    }
    
    memset(process, 0, sizeof(process_t));
    
    /* Initialize process */
    process->pid = scheduler_state.next_pid++;
    strncpy(process->name, name, sizeof(process->name) - 1);
    process->priority = (priority < ULTRAOS_MAX_PRIORITY_LEVELS) ? 
                       (thread_priority_t)priority : PRIORITY_NORMAL;
    process->num_threads = 0;
    process->terminated = false;
    
    /* Create main thread */
    int result = scheduler_create_thread(process->pid, "main", entry_point, NULL, priority);
    if (result < 0) {
        kfree(process);
        return result;
    }
    
    process->main_thread = find_thread_by_tid(result);
    process->threads = process->main_thread;
    process->num_threads = 1;
    
    /* Add to process list */
    process->next = scheduler_state.process_list;
    if (scheduler_state.process_list) {
        scheduler_state.process_list->prev = process;
    }
    scheduler_state.process_list = process;
    
    scheduler_state.stats.active_processes++;
    
    return process->pid;
}

/* Create a new thread */
int scheduler_create_thread(uint32_t pid, const char *name, void *entry_point, 
                           void *arg, uint32_t priority) {
    if (!scheduler_state.initialized || !name || !entry_point) {
        return STATUS_INVALID_PARAMETER;
    }
    
    process_t *process = find_process_by_pid(pid);
    if (!process) {
        return STATUS_INVALID_PID;
    }
    
    if (process->num_threads >= scheduler_state.config.max_threads_per_process) {
        return STATUS_OUT_OF_RESOURCES;
    }
    
    /* Allocate thread control block */
    thread_t *thread = (thread_t *)kmalloc(sizeof(thread_t), GFP_KERNEL);
    if (!thread) {
        return STATUS_OUT_OF_MEMORY;
    }
    
    memset(thread, 0, sizeof(thread_t));
    
    /* Initialize thread */
    thread->tid = scheduler_state.next_tid++;
    thread->pid = pid;
    strncpy(thread->name, name, sizeof(thread->name) - 1);
    thread->state = STATE_READY;
    thread->priority = (priority < ULTRAOS_MAX_PRIORITY_LEVELS) ? 
                      (thread_priority_t)priority : PRIORITY_NORMAL;
    thread->entry_point = entry_point;
    thread->arg = arg;
    thread->time_slice_used = 0;
    thread->total_runtime = 0;
    thread->preemptible = true;
    thread->cpu_affinity = 0;  /* All CPUs initially */
    
    /* Allocate stack */
    if (scheduler_create_thread_stack(thread->tid, ULTRAOS_KERNEL_STACK_SIZE) != STATUS_SUCCESS) {
        kfree(thread);
        return STATUS_OUT_OF_MEMORY;
    }
    
    /* Initialize context (will be setup by context_switch.S) */
    memset(&thread->context, 0, sizeof(thread->context));
    thread->context.rsp = thread->stack_pointer;
    thread->context.rip = (uint64_t)entry_point;
    
    /* Add to ready queue */
    add_to_ready_queue(thread);
    
    /* Add to process thread list */
    thread->next = process->threads;
    if (process->threads) {
        process->threads->prev = thread;
    }
    process->threads = thread;
    process->num_threads++;
    
    scheduler_state.stats.active_threads++;
    
    return thread->tid;
}

/* Terminate a thread */
int scheduler_terminate_thread(uint32_t tid) {
    thread_t *thread = find_thread_by_tid(tid);
    if (!thread) {
        return STATUS_INVALID_TID;
    }
    
    process_t *process = find_process_by_pid(thread->pid);
    if (!process) {
        return STATUS_INVALID_PID;
    }
    
    /* If terminating current thread, schedule next one */
    if (thread == scheduler_state.current_thread) {
        scheduler_state.current_thread = NULL;
        scheduler_schedule();
    }
    
    /* Remove from ready queue */
    remove_from_ready_queue(thread);
    
    /* Remove from process thread list */
    if (thread->prev) {
        thread->prev->next = thread->next;
    } else {
        process->threads = thread->next;
    }
    if (thread->next) {
        thread->next->prev = thread->prev;
    }
    
    process->num_threads--;
    scheduler_state.stats.active_threads--;
    
    /* Free resources */
    free_thread_resources(thread);
    
    return STATUS_SUCCESS;
}

/* Terminate a process */
int scheduler_terminate_process(uint32_t pid) {
    process_t *process = find_process_by_pid(pid);
    if (!process) {
        return STATUS_INVALID_PID;
    }
    
    /* Terminate all threads */
    thread_t *thread = process->threads;
    while (thread) {
        thread_t *next = thread->next;
        scheduler_terminate_thread(thread->tid);
        thread = next;
    }
    
    /* Remove from process list */
    if (process->prev) {
        process->prev->next = process->next;
    } else {
        scheduler_state.process_list = process->next;
    }
    if (process->next) {
        process->next->prev = process->prev;
    }
    
    process->terminated = true;
    scheduler_state.stats.active_processes--;
    
    /* Free process resources */
    free_process_resources(process);
    
    return STATUS_SUCCESS;
}

/* Sleep current thread */
void scheduler_sleep(uint32_t milliseconds) {
    if (!scheduler_state.current_thread) {
        return;
    }
    
    scheduler_state.current_thread->state = STATE_SLEEPING;
    scheduler_state.current_thread->wake_time = scheduler_state.system_time + (milliseconds * 1000000);
    
    /* Add to sleeping list */
    scheduler_state.current_thread->next = scheduler_state.sleeping_threads;
    scheduler_state.current_thread->prev = NULL;
    if (scheduler_state.sleeping_threads) {
        scheduler_state.sleeping_threads->prev = scheduler_state.current_thread;
    }
    scheduler_state.sleeping_threads = scheduler_state.current_thread;
    
    scheduler_schedule();
}

/* Wake up a sleeping thread */
void scheduler_wake_thread(uint32_t tid) {
    thread_t *thread = find_thread_by_tid(tid);
    if (thread && thread->state == STATE_SLEEPING) {
        thread->state = STATE_READY;
        thread->wake_time = 0;
        
        /* Remove from sleeping list */
        if (thread->prev) {
            thread->prev->next = thread->next;
        } else {
            scheduler_state.sleeping_threads = thread->next;
        }
        if (thread->next) {
            thread->next->prev = thread->prev;
        }
        
        /* Add to ready queue */
        add_to_ready_queue(thread);
    }
}

/* Get current thread */
thread_t* scheduler_get_current_thread(void) {
    return scheduler_state.current_thread;
}

/* Get current process */
process_t* scheduler_get_current_process(void) {
    if (!scheduler_state.current_thread) {
        return NULL;
    }
    return find_process_by_pid(scheduler_state.current_thread->pid);
}

/* Set thread priority */
void scheduler_set_priority(uint32_t tid, uint32_t new_priority) {
    thread_t *thread = find_thread_by_tid(tid);
    if (thread) {
        thread->priority = (new_priority < ULTRAOS_MAX_PRIORITY_LEVELS) ? 
                          (thread_priority_t)new_priority : PRIORITY_NORMAL;
    }
}

/* Migrate thread to different CPU */
void scheduler_migrate_thread(uint32_t tid, uint32_t cpu_id) {
    thread_t *thread = find_thread_by_tid(tid);
    if (thread) {
        thread->cpu_affinity = cpu_id;
    }
}

/* Get scheduler statistics */
scheduler_stats_t scheduler_get_stats(void) {
    return scheduler_state.stats;
}

/* Dump scheduler state */
void scheduler_dump_state(void) {
    printk("Scheduler State:\n");
    printk("  Initialized: %s\n", scheduler_state.initialized ? "Yes" : "No");
    printk("  Running: %s\n", scheduler_state.running ? "Yes" : "No");
    printk("  Current Thread: %s (TID: %u)\n", 
           scheduler_state.current_thread ? scheduler_state.current_thread->name : "None",
           scheduler_state.current_thread ? scheduler_state.current_thread->tid : 0U);
    printk("  System Time: %lu ms\n", scheduler_state.system_time / 1000000);
    printk("  Context Switches: %lu\n", scheduler_state.stats.total_context_switches);
}

/* Dump all processes */
void scheduler_dump_processes(void) {
    process_t *process = scheduler_state.process_list;
    printk("Process List:\n");
    
    while (process) {
        printk("  PID: %d, Name: %s, Threads: %d, Priority: %d, Terminated: %s\n",
               process->pid, process->name, process->num_threads, process->priority,
               process->terminated ? "Yes" : "No");
        process = process->next;
    }
}

/* Dump all threads */
void scheduler_dump_threads(void) {
    thread_t *thread = NULL;
    
    for (int priority = 0; priority < ULTRAOS_MAX_PRIORITY_LEVELS; priority++) {
        thread = scheduler_state.ready_queue[priority];
        while (thread) {
            printk("  TID: %d, PID: %d, Name: %s, State: %d, Priority: %d\n",
                   thread->tid, thread->pid, thread->name, thread->state, thread->priority);
            thread = thread->next;
        }
    }
}

/* Private helper functions */

static thread_t* get_next_thread(void) {
    switch (scheduler_state.config.scheduler_algorithm) {
        case 0: /* Round Robin */
            return scheduler_tick_round_robin();
        case 1: /* Priority-based */
            return scheduler_tick_priority();
        case 2: /* CFS */
            return scheduler_tick_cfs();
        case 3: /* Multi-queue */
            return scheduler_tick_multiqueue();
        default:
            return scheduler_tick_round_robin();
    }
}

static void add_to_ready_queue(thread_t *thread) {
    if (!thread || thread->state != STATE_READY) {
        return;
    }
    
    uint32_t priority = thread->priority;
    if (priority >= ULTRAOS_MAX_PRIORITY_LEVELS) {
        priority = ULTRAOS_MAX_PRIORITY_LEVELS - 1;
    }
    
    /* Add to end of priority queue */
    thread->next = NULL;
    thread->prev = scheduler_state.ready_queue[priority];
    
    if (scheduler_state.ready_queue[priority]) {
        scheduler_state.ready_queue[priority]->next = thread;
    }
    scheduler_state.ready_queue[priority] = thread;
}

static void remove_from_ready_queue(thread_t *thread) {
    if (!thread) {
        return;
    }
    
    uint32_t priority = thread->priority;
    if (priority >= ULTRAOS_MAX_PRIORITY_LEVELS) {
        priority = ULTRAOS_MAX_PRIORITY_LEVELS - 1;
    }
    
    if (thread->prev) {
        thread->prev->next = thread->next;
    } else {
        scheduler_state.ready_queue[priority] = thread->next;
    }
    
    if (thread->next) {
        thread->next->prev = thread->prev;
    }
    
    thread->next = NULL;
    thread->prev = NULL;
}

static thread_t* scheduler_tick_round_robin(void) {
    /* Simple round-robin: check all priorities */
    for (int priority = 0; priority < ULTRAOS_MAX_PRIORITY_LEVELS; priority++) {
        thread_t *thread = scheduler_state.ready_queue[priority];
        if (thread) {
            /* Move to next thread in queue */
            scheduler_state.ready_queue[priority] = thread->next;
            if (thread->next) {
                thread->next->prev = NULL;
            }
            thread->next = scheduler_state.ready_queue[priority];
            thread->prev = NULL;
            if (thread->next) {
                thread->next->prev = thread;
            }
            
            return thread;
        }
    }
    return NULL;
}

static thread_t* scheduler_tick_priority(void) {
    /* Priority-based: always pick highest priority ready thread */
    for (int priority = 0; priority < ULTRAOS_MAX_PRIORITY_LEVELS; priority++) {
        thread_t *thread = scheduler_state.ready_queue[priority];
        if (thread) {
            /* Move to next thread in queue */
            scheduler_state.ready_queue[priority] = thread->next;
            if (thread->next) {
                thread->next->prev = NULL;
            }
            thread->next = scheduler_state.ready_queue[priority];
            thread->prev = NULL;
            if (thread->next) {
                thread->next->prev = thread;
            }
            
            return thread;
        }
    }
    return NULL;
}

static thread_t* scheduler_tick_cfs(void) {
    /* Completely Fair Scheduler: pick thread with least runtime */
    thread_t *best_thread = NULL;
    uint64_t min_runtime = UINT64_MAX;
    
    for (int priority = 0; priority < ULTRAOS_MAX_PRIORITY_LEVELS; priority++) {
        thread_t *thread = scheduler_state.ready_queue[priority];
        while (thread) {
            if (thread->total_runtime < min_runtime) {
                min_runtime = thread->total_runtime;
                best_thread = thread;
            }
            thread = thread->next;
        }
    }
    
    if (best_thread) {
        remove_from_ready_queue(best_thread);
    }
    
    return best_thread;
}

static thread_t* scheduler_tick_multiqueue(void) {
    /* Multi-queue: separate queues per priority */
    return scheduler_tick_priority();
}

static void scheduler_update_stats(void) {
    scheduler_state.stats.total_scheduled_threads++;
    scheduler_state.stats.uptime_ms = scheduler_state.system_time / 1000000;
}

static process_t* find_process_by_pid(uint32_t pid) {
    process_t *process = scheduler_state.process_list;
    
    while (process) {
        if (process->pid == pid && !process->terminated) {
            return process;
        }
        process = process->next;
    }
    
    return NULL;
}

static thread_t* find_thread_by_tid(uint32_t tid) {
    process_t *process = scheduler_state.process_list;
    
    while (process) {
        thread_t *thread = process->threads;
        while (thread) {
            if (thread->tid == tid) {
                return thread;
            }
            thread = thread->next;
        }
        process = process->next;
    }
    
    return NULL;
}

static void free_thread_resources(thread_t *thread) {
    if (thread) {
        /* Free stack memory */
        if (thread->stack_base) {
            kfree((void*)thread->stack_base);
        }
        kfree(thread);
    }
}

static void free_process_resources(process_t *process) {
    if (process) {
        kfree(process);
    }
}

/* Context switching functions */
void scheduler_switch_context(thread_t *old_thread, thread_t *new_thread) {
    if (!old_thread || !new_thread) {
        return;
    }
    
    /* Save old context */
    scheduler_save_context(&old_thread->context);
    
    /* Load new context */
    scheduler_restore_context(&new_thread->context);
    
    /* Update thread states */
    old_thread->state = STATE_READY;
    new_thread->state = STATE_RUNNING;
}

void scheduler_save_context(context_frame_t *context) {
    /* This would be implemented in assembly for actual context switching */
    /* For now, just basic implementation */
    __asm__ volatile(
        "pushq %rax\n"
        "pushq %rbx\n"
        "pushq %rcx\n"
        "pushq %rdx\n"
        "pushq %rsi\n"
        "pushq %rdi\n"
        "pushq %r8\n"
        "pushq %r9\n"
        "pushq %r10\n"
        "pushq %r11\n"
        "pushq %r12\n"
        "pushq %r13\n"
        "pushq %r14\n"
        "pushq %r15\n"
        "pushq %rbp\n"
        "pushfq\n"
        "movq %rsp, (%0)\n"
        :
        : "r"(context)
        : "memory"
    );
}

void scheduler_restore_context(context_frame_t *context) {
    /* This would be implemented in assembly for actual context switching */
    /* For now, just basic implementation */
    __asm__ volatile(
        "movq (%0), %%rsp\n"
        "popfq\n"
        "popq %rbp\n"
        "popq %r15\n"
        "popq %r14\n"
        "popq %r13\n"
        "popq %r12\n"
        "popq %r11\n"
        "popq %r10\n"
        "popq %r9\n"
        "popq %r8\n"
        "popq %rdi\n"
        "popq %rsi\n"
        "popq %rdx\n"
        "popq %rcx\n"
        "popq %rbx\n"
        "popq %rax\n"
        :
        : "r"(context)
        : "memory"
    );
}

/* Timer setup */
void scheduler_setup_timers(void) {
    printk("Setting up scheduler timers...\n");
    /* TODO: Implement actual timer setup */
}

/* Shutdown */
void scheduler_shutdown(void) {
    printk("Shutting down scheduler...\n");
    scheduler_stop();
    
    /* Clean up all processes and threads */
    while (scheduler_state.process_list) {
        scheduler_terminate_process(scheduler_state.process_list->pid);
    }
    
    scheduler_state.initialized = false;
    printk("Scheduler shutdown complete\n");
}

/* Process and thread table initialization */
static void process_table_init(void) {
    /* Initialize process table */
    /* This would set up the process table data structure */
}

static void thread_table_init(void) {
    /* Initialize thread table */
    /* This would set up the thread table data structure */
}

/* Thread stack management */
int scheduler_create_thread_stack(uint32_t tid, uint32_t stack_size) {
    thread_t *thread = find_thread_by_tid(tid);
    if (!thread) {
        return STATUS_INVALID_TID;
    }
    
    thread->stack_size = stack_size;
    thread->stack_base = (uint64_t)kmalloc(stack_size, GFP_KERNEL);
    if (!thread->stack_base) {
        return STATUS_OUT_OF_MEMORY;
    }
    
    thread->stack_pointer = thread->stack_base + stack_size - 16; /* 16-byte alignment */
    return STATUS_SUCCESS;
}

int scheduler_destroy_thread_stack(uint32_t tid) {
    thread_t *thread = find_thread_by_tid(tid);
    if (!thread) {
        return STATUS_INVALID_TID;
    }
    
    if (thread->stack_base) {
        kfree((void*)thread->stack_base);
        thread->stack_base = 0;
        thread->stack_pointer = 0;
    }
    
    return STATUS_SUCCESS;
}

/* System time */
uint64_t scheduler_get_system_time(void) {
    return scheduler_state.system_time;
}

/* Memory management integration */
int scheduler_allocate_process_memory(uint32_t pid, uint64_t size) {
    /* TODO: Integrate with virtual memory manager */
    (void)pid;
    (void)size;
    return STATUS_SUCCESS;
}

int scheduler_free_process_memory(uint32_t pid) {
    /* TODO: Integrate with virtual memory manager */
    (void)pid;
    return STATUS_SUCCESS;
}