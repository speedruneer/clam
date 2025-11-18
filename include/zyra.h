#pragma once

#define ZYRA_FUNC_PORT   0x983
#define ZYRA_INDEX_PORT  0x984
#define ZYRA_DATA_PORT   0x985
#define ZYRA_CTRL_PORT   0x988
#define ZYRA_STATUS_PORT 0x987

#define ZYRA_VER_HI 1
#define ZYRA_VER_LO 0

typedef struct zyra_active {
    unsigned int info_flags;
    char device_name[64];
    int device_id;
    unsigned short device_port;
    unsigned char min_zyra_ver_lo;
    unsigned char min_zyra_ver_hi;
} zyra_active_t __attribute__((packed));