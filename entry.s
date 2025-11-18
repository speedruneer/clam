; entry.s - switch to protected mode and jump to kernel

[BITS 16]
global start
extern vesa_mode_info
extern font
extern kernel_size

start:
    cli                 ; disable interrupts

    ; =====================
    ; Setup GDT
    ; =====================
    mov bx, 0x411B
    mov ax, 0x4F01
    mov di, vesa_mode_info
    mov cx, bx
    int 10h
    mov al, [0x3000]
    mov [kernel_size], al

    mov ax, 0x4F02
    int 10h

    mov di, font
	push			ds
	push			es
	;ask BIOS to return VGA bitmap fonts
	mov			ax, 1130h
	mov			bh, 6
	int			10h
	;copy charmap
	push			es
	pop			ds
	pop			es
	mov			si, bp
	mov			cx, 256*16/4
	rep			movsd
	pop			ds

    lgdt [gdt_descriptor]


    ; =====================
    ; Enter protected mode
    ; =====================
    mov eax, cr0
    or eax, 1           ; set PE bit
    mov cr0, eax

    ; Far jump to flush prefetch + load CS
    jmp 0x08:pm_start

gdt_start:
    ; Null descriptor
    dq 0

    ; Code segment: base=0, limit=4GB, exec/read, 32-bit
    dw 0xFFFF           ; limit low
    dw 0                 ; base low
    db 0                 ; base middle
    db 10011010b         ; access byte: present, ring0, code segment, executable, readable
    db 11001111b         ; flags: granularity, 32-bit
    db 0                 ; base high

    ; Data segment: base=0, limit=4GB, read/write
    dd 0x0000FFFF
    db 0
    db 10010010b         ; access: present, ring0, data, writable
    db 11001111b         ; flags: granularity, 32-bit
    db 0

gdt_descriptor:
    dw gdt_descriptor - gdt_start - 1   ; limit
    dd gdt_start                 ; base

[BITS 32]
extern kentry
pm_start:
    ; Setup segment registers
    ; Enable A20 line (simplest version, can skip for emulators)
    in al, 0x92
    or al, 2
    out 0x92, al
    mov al, 0
    mov dx, 0x3F2
    out dx, al
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x7000    ; setup stack somewhere safe

    ; Jump to kernel entry
    call kentry

.hang:
    hlt
    jmp .hang

global set_idt_entry
global load_idt

section .data
    ; IDT: 256 entries, 8 bytes each (2048 bytes)
    idt_table:
        times 256 dq 0

    ; IDT pointer: 16-bit limit + 32-bit base
    idt_ptr:
        dw 2048 - 1         ; limit (size of IDT minus 1)
        dd idt_table        ; base address of the IDT

section .text

; ---------------------------------------------------
; set_idt_entry(int n, void (*handler)(), uint16_t sel, uint8_t flags)
; ---------------------------------------------------
set_idt_entry:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]          ; n
    lea edi, [idt_table + ebx*8]

    mov eax, [ebp+12]         ; handler
    mov word [edi], ax         ; offset_low
    shr eax, 16
    mov word [edi+6], ax       ; offset_high

    mov ax, [ebp+16]          ; selector
    mov [edi+2], ax

    mov byte [edi+4], 0        ; zero
    mov al, [ebp+20]           ; flags
    mov [edi+5], al

    pop ebp
    ret

; ---------------------------------------------------
; load_idt() -- reloads the current IDT
; ---------------------------------------------------
load_idt:
    push ebp
    mov ebp, esp
    lidt [idt_ptr]
    pop ebp
    ret

global isr_stub
isr_stub:
    iret