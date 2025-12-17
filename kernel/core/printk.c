/*
 * UltraOS Printk Implementation
 * Author: zeroday
 * Date: 2025-12-17
 * Version: 2.0
 */

#include "../../include/ultraos_types.h"
#include "../../include/ultraos_config.h"

/* Simple console buffer */
static char console_buffer[ULTRAOS_CONSOLE_BUFFER_SIZE];
static size_t console_pos = 0;

/* Serial port output for x86_64 */
#if defined(__x86_64__) || defined(__x86_64)
static void serial_putchar(char c) {
    /* COM1 port: 0x3F8 */
    #define COM1 0x3F8
    
    /* Wait for transmit buffer to be empty */
    while (!(inb(COM1 + 5) & 0x20));
    
    /* Send character */
    outb(c, COM1);
}

static uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a" (result) : "Nd" (port));
    return result;
}

static void outb(uint8_t value, uint16_t port) {
    __asm__ volatile ("outb %0, %1" : : "a" (value), "Nd" (port));
}
#else
/* Non-x86 architectures: stub implementation */
static void serial_putchar(char c) {
    /* Stub for non-x86 architectures */
    (void)c;
}
#endif

/* Output character to console */
static void console_putchar(char c) {
    /* Add to buffer */
    if (console_pos < ULTRAOS_CONSOLE_BUFFER_SIZE - 1) {
        console_buffer[console_pos++] = c;
        console_buffer[console_pos] = '\0';
    }
    
    /* Output to serial port for debugging */
    serial_putchar(c);
    
    /* Also output to VGA/text mode if available */
    #if defined(__x86_64__) || defined(__x86_64)
    vga_putchar(c);
    #endif
}

/* VGA text mode output (simplified) */
#if defined(__x86_64__) || defined(__x86_64)
static uint16_t* vga_buffer = (uint16_t*)0xB8000;
static size_t vga_pos = 0;

static void vga_putchar(char c) {
    if (c == '\n') {
        /* Move to next line */
        vga_pos = (vga_pos / 80 + 1) * 80;
        if (vga_pos >= 80 * 25) {
            vga_pos = 0; /* Simple scroll - reset to top */
        }
        return;
    }
    
    if (c == '\r') {
        vga_pos = (vga_pos / 80) * 80; /* Move to start of line */
        return;
    }
    
    if (vga_pos < 80 * 25) {
        vga_buffer[vga_pos++] = (0x07 << 8) | c; /* White on black */
    }
}

static void vga_clear(void) {
    for (size_t i = 0; i < 80 * 25; i++) {
        vga_buffer[i] = (0x07 << 8) | ' ';
    }
    vga_pos = 0;
}
#else
static void vga_putchar(char c) {
    (void)c;
}

static void vga_clear(void) {
    /* Stub for non-x86 architectures */
}
#endif

/* Convert integer to string */
static int int_to_str(int num, char* buf, int buf_size, int base, bool negative) {
    int pos = 0;
    int temp;
    char temp_buf[32];
    
    if (num == 0) {
        if (buf_size > 1) {
            buf[0] = '0';
            buf[1] = '\0';
            return 1;
        }
        return 0;
    }
    
    /* Handle negative numbers */
    if (negative && num < 0) {
        buf[pos++] = '-';
        buf_size--;
        num = -num;
    }
    
    /* Convert to string */
    while (num > 0 && pos < buf_size - 1) {
        temp = num % base;
        temp_buf[pos++] = (temp < 10) ? ('0' + temp) : ('a' + temp - 10);
        num /= base;
    }
    
    /* Reverse the string */
    for (int i = 0; i < pos / 2; i++) {
        char temp = temp_buf[i];
        temp_buf[i] = temp_buf[pos - 1 - i];
        temp_buf[pos - 1 - i] = temp;
    }
    
    /* Copy to output buffer */
    for (int i = 0; i < pos && i < buf_size - 1; i++) {
        buf[i] = temp_buf[i];
    }
    buf[pos] = '\0';
    
    return pos;
}

/* Convert unsigned integer to string */
static int uint_to_str(unsigned int num, char* buf, int buf_size, int base) {
    int pos = 0;
    int temp;
    char temp_buf[32];
    
    if (num == 0) {
        if (buf_size > 1) {
            buf[0] = '0';
            buf[1] = '\0';
            return 1;
        }
        return 0;
    }
    
    /* Convert to string */
    while (num > 0 && pos < buf_size - 1) {
        temp = num % base;
        temp_buf[pos++] = (temp < 10) ? ('0' + temp) : ('a' + temp - 10);
        num /= base;
    }
    
    /* Reverse the string */
    for (int i = 0; i < pos / 2; i++) {
        char temp = temp_buf[i];
        temp_buf[i] = temp_buf[pos - 1 - i];
        temp_buf[pos - 1 - i] = temp;
    }
    
    /* Copy to output buffer */
    for (int i = 0; i < pos && i < buf_size - 1; i++) {
        buf[i] = temp_buf[i];
    }
    buf[pos] = '\0';
    
    return pos;
}

/* Convert 64-bit integer to string */
static int int64_to_str(int64_t num, char* buf, int buf_size, int base, bool negative) {
    int pos = 0;
    int64_t temp;
    char temp_buf[32];
    
    if (num == 0) {
        if (buf_size > 1) {
            buf[0] = '0';
            buf[1] = '\0';
            return 1;
        }
        return 0;
    }
    
    /* Handle negative numbers */
    if (negative && num < 0) {
        buf[pos++] = '-';
        buf_size--;
        num = -num;
    }
    
    /* Convert to string */
    while (num > 0 && pos < buf_size - 1) {
        temp = num % base;
        temp_buf[pos++] = (temp < 10) ? ('0' + temp) : ('a' + temp - 10);
        num /= base;
    }
    
    /* Reverse the string */
    for (int i = 0; i < pos / 2; i++) {
        char temp = temp_buf[i];
        temp_buf[i] = temp_buf[pos - 1 - i];
        temp_buf[pos - 1 - i] = temp;
    }
    
    /* Copy to output buffer */
    for (int i = 0; i < pos && i < buf_size - 1; i++) {
        buf[i] = temp_buf[i];
    }
    buf[pos] = '\0';
    
    return pos;
}

/* Convert unsigned 64-bit integer to string */
static int uint64_to_str(uint64_t num, char* buf, int buf_size, int base) {
    int pos = 0;
    uint64_t temp;
    char temp_buf[32];
    
    if (num == 0) {
        if (buf_size > 1) {
            buf[0] = '0';
            buf[1] = '\0';
            return 1;
        }
        return 0;
    }
    
    /* Convert to string */
    while (num > 0 && pos < buf_size - 1) {
        temp = num % base;
        temp_buf[pos++] = (temp < 10) ? ('0' + temp) : ('a' + temp - 10);
        num /= base;
    }
    
    /* Reverse the string */
    for (int i = 0; i < pos / 2; i++) {
        char temp = temp_buf[i];
        temp_buf[i] = temp_buf[pos - 1 - i];
        temp_buf[pos - 1 - i] = temp;
    }
    
    /* Copy to output buffer */
    for (int i = 0; i < pos && i < buf_size - 1; i++) {
        buf[i] = temp_buf[i];
    }
    buf[pos] = '\0';
    
    return pos;
}

/* Print formatted string */
int printk(const char* fmt, ...) {
    va_list args;
    char buffer[256];
    int written = 0;
    const char* p;
    char c;
    
    va_start(args, fmt);
    
    for (p = fmt; *p; p++) {
        if (*p != '%') {
            console_putchar(*p);
            written++;
            continue;
        }
        
        p++; /* Skip '%' */
        
        /* Handle format specifiers */
        switch (*p) {
        case 'c': {
            c = (char)va_arg(args, int);
            console_putchar(c);
            written++;
            break;
        }
        
        case 's': {
            const char* str = va_arg(args, const char*);
            if (str) {
                while (*str) {
                    console_putchar(*str);
                    str++;
                    written++;
                }
            }
            break;
        }
        
        case 'd': {
            int num = va_arg(args, int);
            int len = int_to_str(num, buffer, sizeof(buffer), 10, num < 0);
            for (int i = 0; i < len; i++) {
                console_putchar(buffer[i]);
                written++;
            }
            break;
        }
        
        case 'u': {
            unsigned int num = va_arg(args, unsigned int);
            int len = uint_to_str(num, buffer, sizeof(buffer), 10);
            for (int i = 0; i < len; i++) {
                console_putchar(buffer[i]);
                written++;
            }
            break;
        }
        
        case 'x': {
            unsigned int num = va_arg(args, unsigned int);
            int len = uint_to_str(num, buffer, sizeof(buffer), 16);
            for (int i = 0; i < len; i++) {
                console_putchar(buffer[i]);
                written++;
            }
            break;
        }
        
        case 'X': {
            unsigned int num = va_arg(args, unsigned int);
            int len = uint_to_str(num, buffer, sizeof(buffer), 16);
            for (int i = 0; i < len; i++) {
                console_putchar(buffer[i] >= 'a' ? buffer[i] - 'a' + 'A' : buffer[i]);
                written++;
            }
            break;
        }
        
        case 'p': {
            void* ptr = va_arg(args, void*);
            console_putchar('0');
            console_putchar('x');
            written += 2;
            
            uint64_t addr = (uint64_t)ptr;
            int len = uint64_to_str(addr, buffer, sizeof(buffer), 16);
            for (int i = 0; i < len; i++) {
                console_putchar(buffer[i]);
                written++;
            }
            break;
        }
        
        case 'l': {
            /* Long format specifiers */
            p++; /* Skip 'l' */
            switch (*p) {
            case 'd': {
                long num = va_arg(args, long);
                int len = int_to_str(num, buffer, sizeof(buffer), 10, num < 0);
                for (int i = 0; i < len; i++) {
                    console_putchar(buffer[i]);
                    written++;
                }
                break;
            }
            
            case 'u': {
                unsigned long num = va_arg(args, unsigned long);
                int len = uint_to_str(num, buffer, sizeof(buffer), 10);
                for (int i = 0; i < len; i++) {
                    console_putchar(buffer[i]);
                    written++;
                }
                break;
            }
            
            case 'x': {
                unsigned long num = va_arg(args, unsigned long);
                int len = uint_to_str(num, buffer, sizeof(buffer), 16);
                for (int i = 0; i < len; i++) {
                    console_putchar(buffer[i]);
                    written++;
                }
                break;
            }
            
            case 'l': {
                /* Long long */
                p++; /* Skip second 'l' */
                switch (*p) {
                case 'd': {
                    long long num = va_arg(args, long long);
                    int len = int64_to_str(num, buffer, sizeof(buffer), 10, num < 0);
                    for (int i = 0; i < len; i++) {
                        console_putchar(buffer[i]);
                        written++;
                    }
                    break;
                }
                
                case 'u': {
                    unsigned long long num = va_arg(args, unsigned long long);
                    int len = uint64_to_str(num, buffer, sizeof(buffer), 10);
                    for (int i = 0; i < len; i++) {
                        console_putchar(buffer[i]);
                        written++;
                    }
                    break;
                }
                
                case 'x': {
                    unsigned long long num = va_arg(args, unsigned long long);
                    int len = uint64_to_str(num, buffer, sizeof(buffer), 16);
                    for (int i = 0; i < len; i++) {
                        console_putchar(buffer[i]);
                        written++;
                    }
                    break;
                }
                
                default:
                    console_putchar('%');
                    console_putchar('l');
                    console_putchar('l');
                    written += 3;
                    break;
                }
                break;
            }
            
            default:
                console_putchar('%');
                console_putchar('l');
                written += 2;
                break;
            }
            break;
        }
        
        case '%': {
            console_putchar('%');
            written++;
            break;
        }
        
        default: {
            /* Unknown format specifier */
            console_putchar('%');
            console_putchar(*p);
            written += 2;
            break;
        }
        }
    }
    
    va_end(args);
    return written;
}

/* Simple puts implementation */
int puts(const char* s) {
    int written = 0;
    
    if (!s) {
        return -1;
    }
    
    while (*s) {
        console_putchar(*s);
        s++;
        written++;
    }
    
    console_putchar('\n');
    written++;
    
    return written;
}

/* Get console buffer for debugging */
const char* get_console_buffer(void) {
    return console_buffer;
}

/* Clear console buffer */
void clear_console_buffer(void) {
    console_pos = 0;
    console_buffer[0] = '\0';
    
    #if defined(__x86_64__) || defined(__x86_64)
    vga_clear();
    #endif
}

/* Initialize console */
void console_init(void) {
    clear_console_buffer();
    printk("Console initialized\n");
}