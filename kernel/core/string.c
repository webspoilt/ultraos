/*
 * UltraOS String Utility Functions
 * Author: zeroday
 * Date: 2025-12-17
 * Version: 2.0
 */

#include "../../include/ultraos_types.h"
#include "../../include/ultraos_config.h"

/* Memory operations */
void* memset(void* s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    unsigned char value = (unsigned char)c;
    
    while (n > 0) {
        *p++ = value;
        n--;
    }
    
    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    /* Handle overlapping memory regions */
    if (d < s) {
        /* Copy forward */
        while (n > 0) {
            *d++ = *s++;
            n--;
        }
    } else if (d > s) {
        /* Copy backward */
        d += n;
        s += n;
        while (n > 0) {
            *--d = *--s;
            n--;
        }
    }
    
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    return memcpy(dest, src, n);
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    
    while (n > 0) {
        if (*p1 != *p2) {
            return (*p1 < *p2) ? -1 : 1;
        }
        p1++;
        p2++;
        n--;
    }
    
    return 0;
}

void* memchr(const void* s, int c, size_t n) {
    const unsigned char* p = (const unsigned char*)s;
    unsigned char value = (unsigned char)c;
    
    while (n > 0) {
        if (*p == value) {
            return (void*)p;
        }
        p++;
        n--;
    }
    
    return NULL;
}

void* memrchr(const void* s, int c, size_t n) {
    const unsigned char* p = (const unsigned char*)s + n - 1;
    unsigned char value = (unsigned char)c;
    
    while (n > 0) {
        if (*p == value) {
            return (void*)p;
        }
        p--;
        n--;
    }
    
    return NULL;
}

/* String operations */
size_t strlen(const char* s) {
    size_t len = 0;
    
    if (!s) {
        return 0;
    }
    
    while (s[len] != '\0') {
        len++;
    }
    
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    
    if (!dest || !src) {
        return NULL;
    }
    
    while ((*dest++ = *src++) != '\0') {
        /* Copy characters */
    }
    
    return original_dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* original_dest = dest;
    
    if (!dest || !src) {
        return NULL;
    }
    
    while (n > 0 && (*dest = *src) != '\0') {
        dest++;
        src++;
        n--;
    }
    
    /* Pad with null bytes if needed */
    while (n > 0) {
        *dest++ = '\0';
        n--;
    }
    
    return original_dest;
}

char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    
    if (!dest || !src) {
        return NULL;
    }
    
    /* Find end of destination string */
    while (*dest != '\0') {
        dest++;
    }
    
    /* Append source string */
    while ((*dest++ = *src++) != '\0') {
        /* Copy characters */
    }
    
    return original_dest;
}

char* strncat(char* dest, const char* src, size_t n) {
    char* original_dest = dest;
    
    if (!dest || !src) {
        return NULL;
    }
    
    /* Find end of destination string */
    while (*dest != '\0') {
        dest++;
    }
    
    /* Append up to n characters from source string */
    while (n > 0 && (*dest = *src) != '\0') {
        dest++;
        src++;
        n--;
    }
    
    /* Add null terminator */
    *dest = '\0';
    
    return original_dest;
}

int strcmp(const char* s1, const char* s2) {
    if (!s1 || !s2) {
        return (s1 == s2) ? 0 : (s1 ? 1 : -1);
    }
    
    while (*s1 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    
    if (*s1 == '\0' && *s2 == '\0') {
        return 0;
    } else if (*s1 == '\0') {
        return -1;
    } else if (*s2 == '\0') {
        return 1;
    } else {
        return (*s1 < *s2) ? -1 : 1;
    }
}

int strncmp(const char* s1, const char* s2, size_t n) {
    if (!s1 || !s2) {
        return (s1 == s2) ? 0 : (s1 ? 1 : -1);
    }
    
    while (n > 0 && *s1 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
        n--;
    }
    
    if (n == 0) {
        return 0;
    } else if (*s1 == '\0' && *s2 == '\0') {
        return 0;
    } else if (*s1 == '\0') {
        return -1;
    } else if (*s2 == '\0') {
        return 1;
    } else {
        return (*s1 < *s2) ? -1 : 1;
    }
}

char* strchr(const char* s, int c) {
    const char* p = s;
    char value = (char)c;
    
    if (!s) {
        return NULL;
    }
    
    while (*p != '\0') {
        if (*p == value) {
            return (char*)p;
        }
        p++;
    }
    
    /* Check if we're looking for null terminator */
    if (value == '\0') {
        return (char*)p;
    }
    
    return NULL;
}

char* strrchr(const char* s, int c) {
    const char* last = NULL;
    const char* p = s;
    char value = (char)c;
    
    if (!s) {
        return NULL;
    }
    
    while (*p != '\0') {
        if (*p == value) {
            last = p;
        }
        p++;
    }
    
    /* Check if we're looking for null terminator */
    if (value == '\0') {
        return (char*)p;
    }
    
    return (char*)last;
}

size_t strspn(const char* s, const char* accept) {
    size_t count = 0;
    
    if (!s || !accept) {
        return 0;
    }
    
    while (*s != '\0') {
        const char* p = accept;
        bool found = false;
        
        while (*p != '\0') {
            if (*s == *p) {
                found = true;
                break;
            }
            p++;
        }
        
        if (!found) {
            break;
        }
        
        count++;
        s++;
    }
    
    return count;
}

size_t strcspn(const char* s, const char* reject) {
    size_t count = 0;
    
    if (!s || !reject) {
        return 0;
    }
    
    while (*s != '\0') {
        const char* p = reject;
        bool found = false;
        
        while (*p != '\0') {
            if (*s == *p) {
                found = true;
                break;
            }
            p++;
        }
        
        if (found) {
            break;
        }
        
        count++;
        s++;
    }
    
    return count;
}

char* strpbrk(const char* s, const char* accept) {
    if (!s || !accept) {
        return NULL;
    }
    
    while (*s != '\0') {
        const char* p = accept;
        
        while (*p != '\0') {
            if (*s == *p) {
                return (char*)s;
            }
            p++;
        }
        
        s++;
    }
    
    return NULL;
}

char* strstr(const char* haystack, const char* needle) {
    size_t len_needle, len_haystack;
    
    if (!haystack || !needle) {
        return NULL;
    }
    
    len_needle = strlen(needle);
    len_haystack = strlen(haystack);
    
    if (len_needle > len_haystack) {
        return NULL;
    }
    
    if (len_needle == 0) {
        return (char*)haystack;
    }
    
    for (size_t i = 0; i <= len_haystack - len_needle; i++) {
        if (strncmp(haystack + i, needle, len_needle) == 0) {
            return (char*)(haystack + i);
        }
    }
    
    return NULL;
}

char* strtok(char* s, const char* delim) {
    static char* last = NULL;
    char* token;
    
    if (s == NULL) {
        s = last;
    }
    
    if (s == NULL) {
        return NULL;
    }
    
    /* Skip leading delimiters */
    s += strspn(s, delim);
    
    if (*s == '\0') {
        last = NULL;
        return NULL;
    }
    
    /* Find end of token */
    token = s;
    s = strpbrk(s, delim);
    
    if (s == NULL) {
        last = NULL;
    } else {
        *s = '\0';
        last = s + 1;
    }
    
    return token;
}

char* strdup(const char* s) {
    size_t len;
    char* result;
    
    if (!s) {
        return NULL;
    }
    
    len = strlen(s) + 1;
    result = (char*)kmalloc(len, GFP_KERNEL);
    
    if (result) {
        memcpy(result, s, len);
    }
    
    return result;
}

char* strndup(const char* s, size_t n) {
    size_t len;
    char* result;
    
    if (!s) {
        return NULL;
    }
    
    len = strlen(s);
    if (len > n) {
        len = n;
    }
    
    result = (char*)kmalloc(len + 1, GFP_KERNEL);
    
    if (result) {
        memcpy(result, s, len);
        result[len] = '\0';
    }
    
    return result;
}

/* Character classification functions */
int isalpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isdigit(int c) {
    return (c >= '0' && c <= '9');
}

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int isspace(int c) {
    return (c == ' ' || c == '\t' || c == '\n' || 
            c == '\r' || c == '\v' || c == '\f');
}

int isprint(int c) {
    return (c >= 32 && c <= 126);
}

int isascii(int c) {
    return (c >= 0 && c <= 127);
}

int toupper(int c) {
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 'A';
    }
    return c;
}

int tolower(int c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    return c;
}

/* Safe string functions */
int snprintf(char* str, size_t size, const char* format, ...) {
    va_list args;
    int result;
    
    va_start(args, format);
    result = vsnprintf(str, size, format, args);
    va_end(args);
    
    return result;
}

int vsnprintf(char* str, size_t size, const char* format, va_list args) {
    /* Simplified implementation - just use printk internally */
    char buffer[1024];
    int written = 0;
    
    /* Use a temporary buffer and copy to destination */
    written = vsprintf(buffer, format, args);
    
    if (written < 0) {
        return written;
    }
    
    if (written >= (int)size) {
        if (size > 0) {
            str[0] = '\0';
        }
        return written;
    }
    
    memcpy(str, buffer, written + 1); /* Include null terminator */
    return written;
}

int sprintf(char* str, const char* format, ...) {
    va_list args;
    int result;
    
    va_start(args, format);
    result = vsprintf(str, format, args);
    va_end(args);
    
    return result;
}

int vsprintf(char* str, const char* format, va_list args) {
    /* Simplified implementation - this would need full printf support */
    /* For now, just return 0 */
    (void)str;
    (void)format;
    (void)args;
    return 0;
}