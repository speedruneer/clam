#include <ata.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <rypb.h>


void read_rypb(rypb_t* rypb) {
    if (!rypb) return;

    char* buffer = (char*)malloc(512);
    if (!buffer) return;

    ATA_READ(buffer, 1, 1); // RyPB is LBA 1

    if (strncmp(buffer, "RYPB", 4) != 0) {
        printf("Invalid RyPB signature!\n");
        free(buffer);
        return;
    }

    if ((uint8_t)buffer[4] != 32) {
        printf("Not a 32-bit RyPB!\n");
        free(buffer);
        return;
    }

    size_t offset = 5;
    for (int i = 0; i < 50; i++) {
        if (offset + 10 > 512) break;

        uint32_t start, size;
        uint16_t type;

        memcpy(&start, buffer + offset, 4); offset += 4;
        memcpy(&size,  buffer + offset, 4); offset += 4;
        memcpy(&type,  buffer + offset, 2); offset += 2;

        // Swap endianness
        rypb->partitions[i].starting_lba = start;
        rypb->partitions[i].size         = size;
        rypb->partitions[i].part_type    = type;

        if (rypb->partitions[i].starting_lba == 0 &&
            rypb->partitions[i].size == 0)
            break;
    }

    free(buffer);
}
