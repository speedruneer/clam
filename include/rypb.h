#ifndef RYPB_H
#define RYPB_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum partitions supported in RyPB
#define RYPB_MAX_PARTITIONS 50

// 32-bit partition entry
typedef struct __attribute__((packed)) part32 {
    uint32_t starting_lba;  // 32-bit starting LBA
    uint32_t size;          // 32-bit size
    uint16_t part_type;     // partition type
} rypb_part_32_t;

// RyPB structure (32-bit)
typedef struct __attribute__((packed)) RyPB {
    rypb_part_32_t partitions[RYPB_MAX_PARTITIONS];
} rypb_t;

/**
 * @brief Reads the RyPB (32-bit) from the second sector (LBA 1)
 * @param rypb Pointer to a rypb_t struct to populate
 * @return 0 on success, non-zero on failure
 */
int read_rypb(rypb_t* rypb);

#ifdef __cplusplus
}
#endif

#endif // RYPB_H
