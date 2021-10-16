; This is the kernel's entry point. We could either call main here,
; or we can use this to setup the stack or other nice stuff, like
; perhaps setting up the GDT and segments. Please note that interrupts
; are disabled at this point: More on interrupts later!

[BITS 32]

extern kernel_start		; defined in linker script
extern kernel_end

KERNEL_STACK_SIZE   equ (8<<10)
VIDEO_MEM_ADDR      equ 0xB8000
%define CONFIG_X86_30

; We use a special name to map this section at the begin of our kernel
; =>  Multiboot expects its magic number at the beginning of the kernel.
SECTION .mboot
global start
start:
    jmp stublet

; This part MUST be 4 byte aligned, so we solve that issue using 'ALIGN 4'.
ALIGN 4
mboot:
    ; Multiboot macros to make a few lines more readable later
    MULTIBOOT_PAGE_ALIGN	equ (1 << 0)
    MULTIBOOT_MEMORY_INFO	equ (1 << 1)
    MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
    MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
    MULTIBOOT_CHECKSUM		equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

    ; This is the GRUB Multiboot header. A boot signature
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_CHECKSUM
    dd 0, 0, 0, 0, 0 ; address fields

SECTION .text
ALIGN 4
stublet:
	; Initialize stack pointer
    mov esp, boot_stack
    add esp, KERNEL_STACK_SIZE - 16

	; Interpret multiboot information
    mov DWORD [mb_info], ebx

    ; Initialize CPU features
    call cpu_init

    jmp start32

start32:
	; Jump to the boot processors's C code
    extern main
    call main
    jmp $

%include "asm/prepcpu.s"
%include "asm/gdt.s"
%include "asm/isrs.s"
%include "asm/apic.s"
%include "asm/usermodetask.s"
%include "asm/misc.s"
