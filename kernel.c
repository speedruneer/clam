#include <asm.h>
#include <ata.h>
#include <klib.h>
#include <vesa.h>
#include <text.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <idt.h>
#include <zyra.h>
#include <rypb.h>

mode_info_t vesa_mode_info;
uint8_t font[256][16];
unsigned char kernel_size;

int input_buffer_index = 0;
rypb_t rypb;

void kentry() {
    clear_screen(0, 0, 0);
    set_text_color(255, 255, 255, 0, 0, 0);
    printf("[kernel] VESA works\n");
    init_idt();
    printf("[kernel] reading RyPB\n");
    read_rypb(&rypb);

    while (1) {};
}