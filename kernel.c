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

static const char fs_type_table[256][16] = { "????" };  // Each string up to 15 chars + null

void init_fs_table() {
    strncpy(fs_type_table[88], "RyFS", 16);
    strncpy(fs_type_table[72], "Ext4", 16);
}

void print_rypb_partitions(const rypb_t* rypb) {
    if (!rypb) return;

    printf("RyPB Partitions:\n");
    printf("----------------------------\n");

    for (int i = 0; i < RYPB_MAX_PARTITIONS; i++) {
        const rypb_part_32_t* part = &rypb->partitions[i];

        // Stop printing at empty partition
        if (part->starting_lba == 0 && part->size == 0) break;

        const char* fs_name = "????";
        if (part->part_type < 256) {
            fs_name = fs_type_table[part->part_type];
        }

        printf("Partition %02u:\n", i);
        printf("  Starting LBA: %u\n", part->starting_lba);
        printf("  Size:         %u\n", part->size);
        printf("  Type:         0x%04X (%s)\n", part->part_type, fs_name);
        printf("----------------------------\n");
    }
}

void kentry() {
    clear_screen(0, 0, 0);
    set_text_color(255, 255, 255, 0, 0, 0);
    printf("[kernel] VESA works\n");
    init_idt();
    printf("[kernel] reading RyPB\n");
    read_rypb(&rypb);
    init_fs_table();
    print_rypb_partitions(&rypb);

    while (1) {
    };
}