#ifndef PS2_H
#define PS2_H
#include <stdint.h>
#include <stdbool.h>

extern volatile uint8_t last_key_pressed;
extern volatile bool key_pressed;

struct interrupt_frame
{
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
};

void ps2_keyboard_irq(struct interrupt_frame* frame);

#endif