extern irq_handler
extern get_current_stack
extern finish_task_switch

; Used to realize system calls.
; By entering the handler, the interrupt flag is not cleared.
global isrsyscall
isrsyscall:
cli
push es
push ds
push ebp
push edi
push esi
push edx
push ecx
push ebx
push eax

; Set kernel data segmenets
mov ax, 0x10
mov ds, ax
mov es, ax
mov eax, [esp]
sti

extern syscall_handler
call syscall_handler

cli
add esp, 4 ; eax contains the return value
; => we did not restore eax

pop ebx
pop ecx
pop edx
pop esi
pop edi
pop ebp
pop ds
pop es
sti
iret

; Create a pseudo interrupt on top of the stack.
; Afterwards, we switch to the task with iret.
; We already are in kernel space => no pushing of SS required.
global switch_context
ALIGN 4
switch_context:
mov eax, [esp+4]            ; on the stack is already the address to store the old esp
pushf                       ; push controll register
push DWORD 0x8              ; CS
push DWORD rollback         ; EIP
push DWORD 0x0              ; Interrupt number
push DWORD 0x00edbabe       ; Error code
pusha                       ; push all general purpose registers...
push 0x10                   ; kernel data segment (for ES)
push 0x10                   ; kernel data segment (for DS)

jmp common_switch

ALIGN 4
rollback:
ret

ALIGN 4
common_stub:
pusha
push es
push ds
mov ax, 0x10
mov es, ax
mov ds, ax

; Use the same handler for interrupts and exceptions
push esp
call irq_handler
add esp, 4

cmp eax, 0
je no_context_switch

common_switch:
mov [eax], esp             ; store old esp
call get_current_stack     ; get new esp
xchg eax, esp

; Set task switched flag
mov eax, cr0
or eax, 8
mov cr0, eax

; Set esp0 in the task state segment
extern set_kernel_stack
call set_kernel_stack

; Call cleanup code
call finish_task_switch

no_context_switch:
pop ds
pop es
popa
add esp, 8
iret
