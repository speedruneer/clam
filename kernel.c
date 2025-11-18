#include <asm.h>
#include <ata.h>
#include <klib.h>
#include <vesa.h>
#include <text.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <idt.h>

mode_info_t vesa_mode_info;
uint8_t font[256][16];
unsigned char kernel_size;

char ata_buf[512];

void kentry() {
    clear_screen(0, 0, 0);
    set_text_color(255, 255, 255, 0, 0, 0);
    printf("[kernel] loaded VESA component\n");
    printf("[kernel] Initializing IDT\n");
    init_idt();

    while (1) {};
}