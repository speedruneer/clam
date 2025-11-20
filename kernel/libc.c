#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <vesa.h>
#include <klib.h>
#include <asm.h>

extern char _end;

/* ---------------- Memory functions ---------------- */

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else if (d > s) {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

void *memset(void *pointer, int value, size_t count) {
    uint8_t *p = (uint8_t*)pointer;
    uint8_t byte = (uint8_t)value;  // cast int -> unsigned char

    // Align to machine word boundary
    while (count && ((uintptr_t)p % sizeof(uintptr_t))) {
        *p++ = byte;
        count--;
    }

    // Fill by machine word for speed
    uintptr_t word = 0;
    for (size_t i = 0; i < sizeof(uintptr_t); i++) {
        word = (word << 8) | byte;
    }

    uintptr_t *pw = (uintptr_t*)p;
    while (count >= sizeof(uintptr_t)) {
        *pw++ = word;
        count -= sizeof(uintptr_t);
    }

    // Fill remaining bytes
    p = (uint8_t*)pw;
    while (count--) {
        *p++ = byte;
    }

    return pointer;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = s1, *b = s2;
    for (size_t i = 0; i < n; i++)
        if (a[i] != b[i]) return a[i] - b[i];
    return 0;
}

/* ---------------- String functions ---------------- */

size_t strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return p - s;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strncpy(char *dst, const char *src, size_t n) {
    if (!dst || n == 0) return NULL;
    if (!src) src = "";
    size_t i;
    for (i = 0; i < n - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return dst;
}

char *strcat(char *dest, const char *src) {
    char *d = dest;
    while (*d) d++;
    while ((*d++ = *src++));
    return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (*d) d++;
    size_t i = 0;
    for (; i < n && src[i]; i++) d[i] = src[i];
    d[i] = '\0';
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    size_t i = 0;
    for (; i < n && s1[i] && s1[i] == s2[i]; i++);
    if (i == n) return 0;
    return (unsigned char)s1[i] - (unsigned char)s2[i];
}

char *strchr(const char *s, int c) {
    for (; *s; s++)
        if (*s == (char)c) return (char *)s;
    return c == 0 ? (char *)s : NULL;
}

char *strrchr(const char *s, int c) {
    const char *last = NULL;
    for (; *s; s++)
        if (*s == (char)c) last = s;
    if (c == 0) return (char *)s;
    return (char *)last;
}

char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack, *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return (char *)haystack;
    }
    return NULL;
}

char* strtok(char* str, const char* delim) {
    static char* next;
    if (str) next = str;
    if (!next) return NULL;

    // Skip leading delimiters
    while (*next && strchr(delim, *next)) next++;
    if (!*next) {
        next = NULL;
        return NULL;
    }

    char* token_start = next;

    // Find end of token
    while (*next && !strchr(delim, *next)) next++;
    if (*next) {
        *next = '\0';
        next++;
    } else {
        next = NULL;
    }

    return token_start;
}

// bad but functional allocator

static uint8_t* _heap_start = (uint8_t*)&_end;
#define HEAP_SIZE  0x1000000  // 1 MB heap
#define ALIGN4(x) (((x) + 3) & ~3U)

typedef struct header {
    uint32_t size;          // payload size
    struct header *next;
    uint8_t free;
    uint8_t pad[3];         // padding for alignment
} header_t;

#define HEADER_SIZE (sizeof(header_t))

static const uint32_t _heap_size = HEAP_SIZE;
static header_t *heap_head = NULL;

// initialize heap
void heap_init(void) {
    if (heap_head) return;
    heap_head = (header_t*)_heap_start;
    heap_head->size = _heap_size - HEADER_SIZE;
    heap_head->next = NULL;
    heap_head->free = 1;
}

// convert header <-> payload
static void *header_to_payload(header_t *h) { return (void*)((uint8_t*)h + HEADER_SIZE); }
static header_t *payload_to_header(void *p) { return p ? (header_t*)((uint8_t*)p - HEADER_SIZE) : NULL; }

// split block if it's large enough
static void split_block(header_t *h, uint32_t size) {
    if (h->size <= size + HEADER_SIZE + 4) return; // not enough space to split
    header_t *newh = (header_t*)((uint8_t*)h + HEADER_SIZE + size);
    newh->size = h->size - size - HEADER_SIZE;
    newh->next = h->next;
    newh->free = 1;
    h->size = size;
    h->next = newh;
}

// coalesce consecutive free blocks
static void coalesce(void) {
    header_t *cur = heap_head;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += HEADER_SIZE + cur->next->size;
            cur->next = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}

// find first free block large enough
static header_t *find_free_block(uint32_t size) {
    header_t *cur = heap_head;
    while (cur) {
        if (cur->free && cur->size >= size) return cur;
        cur = cur->next;
    }
    return NULL;
}

// --------------------
// Public API
// --------------------

void *malloc(size_t size) {
    if (!size) {printf("[malloc] !size\n"); return NULL;}
    heap_init();
    uint32_t req = ALIGN4(size);
    header_t *blk = find_free_block(req);
    if (!blk) {printf("[malloc] !blk\n"); return NULL;} // out of memory
    split_block(blk, req);
    blk->free = 0;
    printf("[malloc] allocated %d bytes\n", size);
    return header_to_payload(blk);
}

void free(void *ptr) {
    if (!ptr) return;
    header_t *h = payload_to_header(ptr);
    printf("[free] freed %d bytes\n", h->size);
    if ((uint8_t*)h < _heap_start || (uint8_t*)h >= _heap_start + _heap_size) return;
    h->free = 1;
    coalesce();
}

void *realloc(void *ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) { free(ptr); return NULL; }

    header_t *h = payload_to_header(ptr);
    uint32_t req = ALIGN4(size);

    if (h->size >= req) {
        split_block(h, req);
        return ptr;
    }

    // try to extend into next block if free
    if (h->next && h->next->free && h->size + HEADER_SIZE + h->next->size >= req) {
        h->size += HEADER_SIZE + h->next->size;
        h->next = h->next->next;
        split_block(h, req);
        return ptr;
    }

    // otherwise, allocate new and copy
    void *newp = malloc(size);
    if (!newp) return NULL;
    size_t copy_size = h->size < req ? h->size : req;
    memcpy(newp, ptr, copy_size);
    free(ptr);
    return newp;
}

void *calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void *p = malloc(total);
    if (!p) return NULL;
    memset(p, 0, total);
    return p;
}
