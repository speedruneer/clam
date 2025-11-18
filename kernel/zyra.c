// todo: zyra 1.1+ support

#include <asm.h>
#include <zyra.h>

zyra_active_t* zyra_get_device_info(unsigned short port) {
    zyra_active_t info = *zyra_get_device_info(port);
    outb(ZYRA_FUNC_PORT, 0x65);
    outw(ZYRA_INDEX_PORT, port);
    outd(ZYRA_DATA_PORT, (uint32_t)&info);
    outb(ZYRA_CTRL_PORT, 0x1);
    return &info;
}

int zyra_send_data(unsigned short port, void* data) {
    zyra_active_t info = *zyra_get_device_info(port);
    if ((info.info_flags & 1) == 0) return;
    if (info.min_zyra_ver_hi < ZYRA_VER_HI) return;
    if ((info.min_zyra_ver_hi == ZYRA_VER_HI) && (info.min_zyra_ver_lo < ZYRA_VER_LO)) return;
    outb(ZYRA_FUNC_PORT, 0x66);
    outw(ZYRA_INDEX_PORT, port);
    outd(ZYRA_DATA_PORT, (uint32_t)data);
    outb(ZYRA_CTRL_PORT, 0x8);
    uint8_t status = inb(ZYRA_STATUS_PORT);
    return status;
}