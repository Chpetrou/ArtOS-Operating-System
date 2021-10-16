; This will set up the x86 control registers:
; Caching and the floating point unit are enabled
; Bootstrap page tables are loaded and page size
; extensions (huge pages) enabled.
global cpu_init
cpu_init:

; initialize page tables

; map vga 1:1
push edi
mov eax, VIDEO_MEM_ADDR   ; map vga
and eax, 0xFFFFF000       ; page align lower half
mov edi, eax

shr edi, 10               ; (edi >> 12) * 4 (index for boot_pgt)

add edi, boot_pgt
or eax, 0x113             ; set present, global, writable and cache disable bits
mov DWORD [edi], eax
pop edi

; map multiboot info 1:1
push edi
mov eax, DWORD [mb_info]  ; map multiboot info
and eax, 0xFFFFF000       ; page align lower half
mov edi, eax

shr edi, 10               ; (edi >> 12) * 4 (index for boot_pgt)

add edi, boot_pgt
or eax, 0x101             ; set present and global bits
mov DWORD [edi], eax
pop edi

; map kernel 1:1
push edi
push ebx
push ecx
mov ecx, kernel_start
mov ebx, kernel_end
add ebx, 0x1000
L0: cmp ecx, ebx
jae L1
mov eax, ecx
and eax, 0xFFFFF000       ; page align lower half
mov edi, eax

shr edi, 10               ; (edi >> 12) * 4 (index for boot_pgt)

add edi, boot_pgt
or eax, 0x103             ; set present, global and writable bits
mov DWORD [edi], eax
add ecx, 0x1000
jmp L0
L1:
pop ecx
pop ebx
pop edi

; Set CR3
mov eax, boot_pgd
mov cr3, eax

; Set CR4
mov eax, cr4
and eax, 0xfffbf9ff     ; disable SSE
or eax, (1 << 4)        ; enable PSE
mov cr4, eax

; Set CR0
mov eax, cr0
and eax, ~(1 << 30)     ; enable caching
and eax, ~(1 << 16)    ; allow kernel write access to read-only pages
or eax, (1 << 31)       ; enable paging
mov cr0, eax

ret

; there is no long mode
Linvalid:
jmp $
