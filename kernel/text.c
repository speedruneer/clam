// text.c -- 8x16 text renderer with printable ASCII font and printf
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

// If you have real headers, include them instead.
#include <text.h>
#include <vesa.h>

extern uint8_t font[256][16];

// ===== configuration =====
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t fg_r = 255, fg_g = 255, fg_b = 255;
static uint8_t bg_r =   0, bg_g =   0, bg_b =   0;

static int max_cols = 0;
static int max_rows = 0;

static void update_max(void) {
    if (vesa_mode_info.XResolution <= 0 || vesa_mode_info.YResolution <= 0) {
        max_cols = 80;
        max_rows = 25;
    } else {
        max_cols = vesa_mode_info.XResolution / CHAR_WIDTH;
        max_rows = vesa_mode_info.YResolution / CHAR_HEIGHT;
    }
}

// ===== drawing primitives =====
void draw_char(int cx, int cy, char c) {
    update_max();
    if ((unsigned char)c < 32 || (unsigned char)c > 126) c = '?';
    const uint8_t *bitmap = font[(unsigned char)c];

    int px = cx * CHAR_WIDTH;
    int py = cy * CHAR_HEIGHT;

    for (int row = 0; row < CHAR_HEIGHT; ++row) {
        uint8_t bits = bitmap[row];
        for (int col = 0; col < CHAR_WIDTH; ++col) {
            // bit7 is leftmost pixel
            if (bits & (1u << (7 - col))) {
                set_pixel(px + col, py + row, fg_r, fg_g, fg_b);
            } else {
                set_pixel(px + col, py + row, bg_r, bg_g, bg_b);
            }
        }
    }
}

static void newline_advance(void) {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= max_rows) {
        // simple scroll behavior: clear screen when full
        clear_screen(bg_r, bg_g, bg_b);
        cursor_x = 0;
        cursor_y = 0;
    }
}

void print(const char *s) {
    if (!s) return;
    update_max();
    while (*s) {
        if (*s == '\n') {
            newline_advance();
        } else if (*s == '\r') {
            cursor_x = 0;
        } else {
            draw_char(cursor_x, cursor_y, *s);
            cursor_x++;
            if (cursor_x >= max_cols) newline_advance();
        }
        ++s;
    }
}

void clear_screen_text(void) {
    clear_screen(bg_r, bg_g, bg_b);
    cursor_x = 0;
    cursor_y = 0;
    update_max();
}

void set_text_color(uint8_t fr, uint8_t fg, uint8_t fb,
                    uint8_t br, uint8_t bg, uint8_t bb) {
    fg_r = fr; fg_g = fg; fg_b = fb;
    bg_r = br; bg_g = bg; bg_b = bb;
}

static void utoa_buf(unsigned int value, unsigned int base, int uppercase,
                     char *out, size_t out_sz)
{
    const char *digits = uppercase ?
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" :
        "0123456789abcdefghijklmnopqrstuvwxyz";

    char tmp[33];
    int pos = 0;

    if (value == 0) {
        if (out_sz > 1) {out[0] = '0'; out[1] = '\0';}
        return;
    }

    unsigned int orig = value;

    // Normal conversion
    while (value && pos < (int)sizeof(tmp)-1) {
        tmp[pos++] = digits[value % base];
        value /= base;
    }

    // Now tmp[] has reversed string; pos = length

    // Reverse into out first
    int out_pos = 0;
    for (int i = pos - 1; i >= 0 && out_pos + 1 < (int)out_sz; i--) {
        out[out_pos++] = tmp[i];
    }
    out[out_pos] = '\0';

    if (base != 16) return;

    // Strip leading zeros
    int start = 0;
    while (out[start] == '0' && out[start + 1] != '\0') {
        start++;
    }

    if (start > 0) {
        int len = 0;
        while (out[start + len]) {
            out[len] = out[start + len];
            len++;
        }
        out[len] = '\0';
    }

    // If one digit but value < 0x100, pad to two-digit hex
    if (out[1] == '\0' && orig < 0x100) {
        if (out_sz > 2) {
            out[2] = '\0';
            out[1] = out[0];
            out[0] = '0';
        }
    }
}

static void print_unsigned_ll(unsigned int v, int base, int uppercase) {
    char buf[33];
    utoa_buf(v, base, uppercase, buf, sizeof(buf));
    print(buf);
}

static void print_signed_intint(int v, int base) {
    if (v < 0) {
        print("-");
        print_unsigned_ll((unsigned int)(-v), base, 0);
    } else {
        print_unsigned_ll((unsigned int)v, base, 0);
    }
}
// ===== vprintf/printf =====
// Supports: %c %s %d %i %u %x %X %p %%
// Flags: 0 -
// Width: any number (e.g. 03, 8, 16, etc)
// Length: l for long (ignored since you're using 32-bit anyway)

static void pad_print(int width, int len, int left_align, char padchar) {
    int pad = width - len;
    if (pad < 0) pad = 0;

    if (!left_align) {
        while (pad--) {
            char c[2] = { padchar, 0 };
            print(c);
        }
    }

    // caller prints actual content

    if (left_align) {
        while (pad--) {
            char c[2] = { ' ', 0 };
            print(c);
        }
    }
}

static void vprintf_internal(const char *fmt, va_list args) {
    while (*fmt) {
        if (*fmt != '%') {
            char c[2] = {*fmt, 0};
            print(c);
            fmt++;
            continue;
        }

        fmt++;  // skip '%'

        // ===== flags =====
        int left_align = 0;
        int pad_zero = 0;

        int parsing = 1;
        while (parsing) {
            switch (*fmt) {
                case '-': left_align = 1; fmt++; break;
                case '0': pad_zero = 1; fmt++; break;
                default: parsing = 0; break;
            }
        }

        // ===== width =====
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        // ===== length =====
        int longflag = 0;
        if (*fmt == 'l') {
            longflag = 1;
            fmt++;
        }

        // ===== specifier =====
        char conv = *fmt;
        if (!conv) break;

        fmt++;

        char buf[64];

        switch (conv) {

        case 'c': {
            char ch = (char)va_arg(args, int);
            buf[0] = ch;
            buf[1] = 0;
            int len = 1;
            pad_print(width, len, left_align, pad_zero ? '0' : ' ');
            print(buf);
            if (left_align) pad_print(width, len, 1, ' ');
            break;
        }

        case 's': {
            char *s = va_arg(args, char *);
            if (!s) s = "(null)";
            int len = 0;
            const char *p = s;
            while (*p++) len++;
            pad_print(width, len, left_align, pad_zero ? '0' : ' ');
            print(s);
            if (left_align) pad_print(width, len, 1, ' ');
            break;
        }

        case 'd':
        case 'i': {
            int n = va_arg(args, int);
            print_signed_intint(n, 10);
            break;
        }

        case 'u': {
            unsigned int n = va_arg(args, unsigned int);
            utoa_buf(n, 10, 0, buf, sizeof(buf));
            int len = 0; for (char *p = buf; *p; p++) len++;
            pad_print(width, len, left_align, pad_zero ? '0' : ' ');
            print(buf);
            if (left_align) pad_print(width, len, 1, ' ');
            break;
        }

        case 'x':
        case 'X': {
            unsigned int n = va_arg(args, unsigned int);
            utoa_buf(n, 16, conv == 'X', buf, sizeof(buf));

            int len = 0; for (char *p = buf; *p; p++) len++;

            pad_print(width, len, left_align, pad_zero ? '0' : ' ');
            print(buf);
            if (left_align) pad_print(width, len, 1, ' ');
            break;
        }

        case 'p': {
            uintptr_t p = (uintptr_t)va_arg(args, void *);
            print("0x");
            utoa_buf(p, 16, 0, buf, sizeof(buf));
            print(buf);
            break;
        }

        case '%': {
            print("%");
            break;
        }

        default: {
            char weird[3] = {'%', conv, 0};
            print(weird);
            break;
        }
        }
    }
}

void printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf_internal(fmt, ap);
    va_end(ap);
}

void println(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf_internal(fmt, ap);
    va_end(ap);
    print("\n");
}

// Minimal stack protector stub if your build needs it:
void __stack_chk_fail_local(void) {
    for (;;);
}
