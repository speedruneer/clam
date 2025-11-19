; bootloader.s - loads N sectors from disk to 0x9000 and jumps there

[org 0x7c00]        ; BIOS loads bootloader here
[bits 16]
%define LOAD_SEG 0x9000    ; segment where kernel will be loaded

start:
    cli                 ; disable interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov bx, LOAD_SEG
    mov dh, 0           ; head
    mov dl, 0x80        ; first hard disk
    xor cx, cx

    mov si, ERROR_MSG

    mov al, KERNEL_SECTORS     ; number of sectors to read
    mov [0x3000], al
    mov ch, 0           ; cylinder 0
    mov cl, 3           ; starting sector (sector 1 is bootloader)
    
    call read_sectors
    jc disk_error       ; jump if carry set

    jmp 0x0000:0x9000        ; jump to loaded kernel

; -----------------------------
; BIOS disk read routine
; Inputs:
;   ES:BX - destination segment:offset
;   DL    - disk number
;   CH    - cylinder
;   CL    - sector
;   DH    - head
;   CX    - number of sectors
; Sets CF on error
; -----------------------------
read_sectors:
    mov ah, 0x02        ; BIOS read sectors
    int 0x13
    ret

disk_error:
    mov ah, 0x0e        ; teletype BIOS print
.print_char:
    lodsb
    cmp al, 0
    je .hang
    int 0x10
    jmp .print_char

.hang:
    hlt
    jmp .hang

ERROR_MSG: db "Disk read error!", 0


times ((510) - ($-$$)) db 0

dw 0xAA55

db "RYPB", 32

dd (KERNEL_SECTORS+3)
dd 1024
dw 88

dd (KERNEL_SECTORS+3+1025)
dd 2048
dw 0072


times 1024 - ($ - $$) db 0