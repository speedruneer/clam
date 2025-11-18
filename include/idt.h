#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <asm.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PIC1        0x20
#define PIC2        0xA0
#define PIC1_COMMAND    PIC1
#define PIC1_DATA   (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA   (PIC2+1)
#define PIC_EOI     0x20

#define ICW1 0x11
#define ICW4 0x01

// C-compatible NASM functions
extern void set_idt_entry(int n, void (*handler)(), uint16_t sel, uint8_t flags);
extern void load_idt(void);
extern void isr_stub(void);
void init_idt();

#ifdef __cplusplus
}
#endif


#endif
