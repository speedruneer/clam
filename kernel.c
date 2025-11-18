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
#include <ps2.h>
#include <zyra.h>

volatile uint8_t last_key_pressed = 0;
volatile bool key_pressed = false;

mode_info_t vesa_mode_info;
uint8_t font[256][16];
unsigned char kernel_size;

char ata_buf[512];

void kentry() {
    clear_screen(0, 0, 0);
    set_text_color(255, 255, 255, 0, 0, 0);
    printf("[kernel] VESA works\n");
    init_idt();

    while (1) {
        if (key_pressed == true) {
            printf("Key pressed: %c\n", last_key_pressed);
            key_pressed = false;
        }
    };
}