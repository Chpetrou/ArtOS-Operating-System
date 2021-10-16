SECTION .data

global mb_info:
ALIGN 8
mb_info:
DQ 0

ALIGN 4096
global boot_stack
boot_stack:
TIMES (KERNEL_STACK_SIZE) DB 0xcd

; Bootstrap page tables are used during the initialization.
ALIGN 4096

boot_pgd:

DD boot_pgt + 0x107    ; PG_PRESENT | PG_GLOBAL | PG_RW | PG_USER
times 1022 DD 0        ; PAGE_MAP_ENTRIES - 2
DD boot_pgd + 0x303 ; PG_PRESENT | PG_GLOBAL | PG_RW | PG_SELF (self-reference)
boot_pgt:
times 1024 DD 0

; add some hints to the ELF file
SECTION .note.GNU-stack noalloc noexec nowrite progbits
