#include <stdint.h>
#include <stdbool.h>
#include <asm.h>
#include <ps2.h>

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64

// IRQ1 handler for keyboard
void ps2_keyboard_irq(struct interrupt_frame* frame) {
    uint8_t scancode = inb(KBD_DATA_PORT);

    // Check the high bit: if set, key released; if clear, key pressed
    if (scancode & 0x80) {
        key_pressed = false;         // key released
    } else {
        last_key_pressed = scancode; // key pressed
        key_pressed = true;
    }

    // Send EOI to PIC (assuming legacy PIC)
    outb(0x20, 0x20);  // master PIC
}
