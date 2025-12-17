/**
 * @file    kernel/arch/x86_64/interrupts.c
 * @brief   Interrupt handling implementation for x86_64 architecture
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
#include <kernel/arch/x86_64/cpu.h>

// Interrupt Descriptor Table (IDT) entries
static idt_entry_t idt_entries[256] __aligned(16);
static idtr_t idtr;

// External interrupt handler declarations
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

// ISR handler pointers
static isr_handler_t isr_handlers[32] = {NULL};

// IRQ handler pointers
static irq_handler_t irq_handlers[16] = {NULL};

/**
 * @brief Initialize the Interrupt Descriptor Table (IDT)
 * @return status_t Status of initialization
 */
status_t interrupts_init(void)
{
    printk("Initializing interrupt handling...\n");
    
    // Zero out the IDT entries
    memset(idt_entries, 0, sizeof(idt_entries));
    
    // Install interrupt service routines (ISRs)
    interrupts_set_isr(0,  (uintptr_t)isr0);
    interrupts_set_isr(1,  (uintptr_t)isr1);
    interrupts_set_isr(2,  (uintptr_t)isr2);
    interrupts_set_isr(3,  (uintptr_t)isr3);
    interrupts_set_isr(4,  (uintptr_t)isr4);
    interrupts_set_isr(5,  (uintptr_t)isr5);
    interrupts_set_isr(6,  (uintptr_t)isr6);
    interrupts_set_isr(7,  (uintptr_t)isr7);
    interrupts_set_isr(8,  (uintptr_t)isr8);
    interrupts_set_isr(9,  (uintptr_t)isr9);
    interrupts_set_isr(10, (uintptr_t)isr10);
    interrupts_set_isr(11, (uintptr_t)isr11);
    interrupts_set_isr(12, (uintptr_t)isr12);
    interrupts_set_isr(13, (uintptr_t)isr13);
    interrupts_set_isr(14, (uintptr_t)isr14);
    interrupts_set_isr(15, (uintptr_t)isr15);
    interrupts_set_isr(16, (uintptr_t)isr16);
    interrupts_set_isr(17, (uintptr_t)isr17);
    interrupts_set_isr(18, (uintptr_t)isr18);
    interrupts_set_isr(19, (uintptr_t)isr19);
    interrupts_set_isr(20, (uintptr_t)isr20);
    interrupts_set_isr(21, (uintptr_t)isr21);
    interrupts_set_isr(22, (uintptr_t)isr22);
    interrupts_set_isr(23, (uintptr_t)isr23);
    interrupts_set_isr(24, (uintptr_t)isr24);
    interrupts_set_isr(25, (uintptr_t)isr25);
    interrupts_set_isr(26, (uintptr_t)isr26);
    interrupts_set_isr(27, (uintptr_t)isr27);
    interrupts_set_isr(28, (uintptr_t)isr28);
    interrupts_set_isr(29, (uintptr_t)isr29);
    interrupts_set_isr(30, (uintptr_t)isr30);
    interrupts_set_isr(31, (uintptr_t)isr31);
    
    // Install interrupt requests (IRQs)
    interrupts_set_irq(0,  (uintptr_t)irq0);
    interrupts_set_irq(1,  (uintptr_t)irq1);
    interrupts_set_irq(2,  (uintptr_t)irq2);
    interrupts_set_irq(3,  (uintptr_t)irq3);
    interrupts_set_irq(4,  (uintptr_t)irq4);
    interrupts_set_irq(5,  (uintptr_t)irq5);
    interrupts_set_irq(6,  (uintptr_t)irq6);
    interrupts_set_irq(7,  (uintptr_t)irq7);
    interrupts_set_irq(8,  (uintptr_t)irq8);
    interrupts_set_irq(9,  (uintptr_t)irq9);
    interrupts_set_irq(10, (uintptr_t)irq10);
    interrupts_set_irq(11, (uintptr_t)irq11);
    interrupts_set_irq(12, (uintptr_t)irq12);
    interrupts_set_irq(13, (uintptr_t)irq13);
    interrupts_set_irq(14, (uintptr_t)irq14);
    interrupts_set_irq(15, (uintptr_t)irq15);
    
    // Load the IDT
    idtr.base = (uintptr_t)idt_entries;
    idtr.limit = sizeof(idt_entries) - 1;
    
    __asm__ volatile("lidt %0" : : "m" (idtr));
    
    // Enable interrupts
    __asm__ volatile("sti");
    
    printk("Interrupt handling initialized successfully\n");
    
    return STATUS_SUCCESS;
}

/**
 * @brief Set up an interrupt service routine (ISR) entry in the IDT
 * @param interrupt Interrupt number (0-31)
 * @param handler_ptr Pointer to the interrupt handler function
 */
void interrupts_set_isr(uint8_t interrupt, uintptr_t handler_ptr)
{
    if (interrupt > 31) {
        return;
    }
    
    idt_entries[interrupt].offset_low = (uint16_t)(handler_ptr & 0xFFFF);
    idt_entries[interrupt].selector = 0x08;  // Code segment selector
    idt_entries[interrupt].zero = 0x00;
    idt_entries[interrupt].type_attr = 0x8E;  // Interrupt gate, DPL=0, present
    idt_entries[interrupt].offset_middle = (uint16_t)((handler_ptr >> 16) & 0xFFFF);
    idt_entries[interrupt].offset_high = (uint32_t)(handler_ptr >> 32);
    idt_entries[interrupt].reserved = 0x00;
}

/**
 * @brief Set up an interrupt request (IRQ) entry in the IDT
 * @param irq IRQ number (0-15)
 * @param handler_ptr Pointer to the IRQ handler function
 */
void interrupts_set_irq(uint8_t irq, uintptr_t handler_ptr)
{
    if (irq > 15) {
        return;
    }
    
    uint8_t interrupt = 32 + irq;  // IRQs start at interrupt 32
    
    idt_entries[interrupt].offset_low = (uint16_t)(handler_ptr & 0xFFFF);
    idt_entries[interrupt].selector = 0x08;  // Code segment selector
    idt_entries[interrupt].zero = 0x00;
    idt_entries[interrupt].type_attr = 0x8E;  // Interrupt gate, DPL=0, present
    idt_entries[interrupt].offset_middle = (uint16_t)((handler_ptr >> 16) & 0xFFFF);
    idt_entries[interrupt].offset_high = (uint32_t)(handler_ptr >> 32);
    idt_entries[interrupt].reserved = 0x00;
}

/**
 * @brief Common interrupt service routine handler
 * @param int_no Interrupt number
 * @param err_code Error code (if applicable)
 */
void isr_handler(uint32_t int_no, uint32_t err_code)
{
    printk("Exception #%u occurred\n", int_no);
    
    if (isr_handlers[int_no] != NULL) {
        isr_handlers[int_no](int_no, err_code);
    } else {
        // Default exception handler
        interrupts_handle_exception(int_no, err_code);
    }
}

/**
 * @brief Common interrupt request handler
 * @param irq IRQ number (0-15)
 */
void irq_handler(uint8_t irq)
{
    // Send End of Interrupt (EOI) to PIC
    if (irq >= 8) {
        // Slave PIC
        outb(0xA0, 0x20);
    }
    // Master PIC
    outb(0x20, 0x20);
    
    if (irq_handlers[irq] != NULL) {
        irq_handlers[irq](irq);
    } else {
        printk("Unhandled IRQ %u\n", irq);
    }
}

/**
 * @brief Register an ISR handler
 * @param interrupt Interrupt number
 * @param handler Pointer to the handler function
 * @return status_t Status of registration
 */
status_t interrupts_register_isr(uint8_t interrupt, isr_handler_t handler)
{
    if (interrupt > 31) {
        return STATUS_INVALID_PARAMETER;
    }
    
    isr_handlers[interrupt] = handler;
    return STATUS_SUCCESS;
}

/**
 * @brief Register an IRQ handler
 * @param irq IRQ number
 * @param handler Pointer to the handler function
 * @return status_t Status of registration
 */
status_t interrupts_register_irq(uint8_t irq, irq_handler_t handler)
{
    if (irq > 15) {
        return STATUS_INVALID_PARAMETER;
    }
    
    irq_handlers[irq] = handler;
    return STATUS_SUCCESS;
}

/**
 * @brief Default exception handler
 * @param int_no Interrupt number
 * @param err_code Error code
 */
void interrupts_handle_exception(uint32_t int_no, uint32_t err_code)
{
    printk("CRITICAL: Unhandled exception %u with error code 0x%08x\n", int_no, err_code);
    printk("Halting system for security reasons\n");
    
    // Disable interrupts
    __asm__ volatile("cli");
    
    // Halt the system
    while (1) {
        __asm__ volatile("hlt");
    }
}

/**
 * @brief Enable hardware interrupts
 */
void interrupts_enable(void)
{
    __asm__ volatile("sti");
}

/**
 * @brief Disable hardware interrupts
 */
void interrupts_disable(void)
{
    __asm__ volatile("cli");
}

/**
 * @brief Send End of Interrupt (EOI) to PIC
 * @param irq IRQ number
 */
void interrupts_send_eoi(uint8_t irq)
{
    if (irq >= 8) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

/**
 * @brief Program the PIC (8259) for proper IRQ handling
 */
void pic_init(void)
{
    // Initialize PIC 1 (Master)
    outb(0x20, 0x11);  // ICW1: Initialize + ICW4 needed
    outb(0x21, 0x20);  // ICW2: IRQ base at 32 (0x20)
    outb(0x21, 0x04);  // ICW3: Slave on IRQ2
    outb(0x21, 0x01);  // ICW4: 8086 mode
    
    // Initialize PIC 2 (Slave)
    outb(0xA0, 0x11);  // ICW1: Initialize + ICW4 needed
    outb(0xA1, 0x28);  // ICW2: IRQ base at 40 (0x28)
    outb(0xA1, 0x02);  // ICW3: Slave ID 2
    outb(0xA1, 0x01);  // ICW4: 8086 mode
    
    // Disable all IRQs initially
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
    
    printk("PIC initialized\n");
}

/**
 * @brief Set IRQ mask for master PIC
 * @param mask IRQ mask value
 */
void pic_set_mask_master(uint8_t mask)
{
    outb(0x21, mask);
}

/**
 * @brief Set IRQ mask for slave PIC
 * @param mask IRQ mask value
 */
void pic_set_mask_slave(uint8_t mask)
{
    outb(0xA1, mask);
}

/**
 * @brief Get IRQ mask from master PIC
 * @return uint8_t Current IRQ mask
 */
uint8_t pic_get_mask_master(void)
{
    return inb(0x21);
}

/**
 * @brief Get IRQ mask from slave PIC
 * @return uint8_t Current IRQ mask
 */
uint8_t pic_get_mask_slave(void)
{
    return inb(0xA1);
}