; The first 32 interrupt service routines (ISR) entries correspond to exceptions.
; Some exceptions will push an error code onto the stack which is specific to
; the exception caused. To decrease the complexity, we handle this by pushing a
; Dummy error code of 0 onto the stack for any ISR that doesn't push an error
; code already.
;
; ISRs are registered as "Interrupt Gate".
; Therefore, the interrupt flag (IF) is already cleared.

; NASM macro which pushs also an pseudo error code
%macro isrstub_pseudo_error 1
global isr%1
isr%1:
push byte 0 ; pseudo error code
push byte %1
jmp common_stub
%endmacro

; Similar to isrstub_pseudo_error, but without pushing
; a pseudo error code => The error code is already
; on the stack.
%macro isrstub 1
global isr%1
isr%1:
push byte %1
jmp common_stub
%endmacro

; Create isr entries, where the number after the
; pseudo error code represents following interrupts:
; 0: Divide By Zero Exception
; 1: Debug Exception
; 2: Non Maskable Interrupt Exception
; 3: Int 3 Exception
; 4: INTO Exception
; 5: Out of Bounds Exception
; 6: Invalid Opcode Exception
; 7: Coprocessor Not Available Exception
%assign i 0
%rep    8
isrstub_pseudo_error i
%assign i i+1
%endrep

; 8: Double Fault Exception (With Error Code!)
isrstub 8

; 9: Coprocessor Segment Overrun Exception
isrstub_pseudo_error 9

; 10: Bad TSS Exception (With Error Code!)
; 11: Segment Not Present Exception (With Error Code!)
; 12: Stack Fault Exception (With Error Code!)
; 13: General Protection Fault Exception (With Error Code!)
; 14: Page Fault Exception (With Error Code!)
%assign i 10
%rep 5
isrstub i
%assign i i+1
%endrep

; 15: Reserved Exception
; 16: Floating Point Exception
; 17: Alignment Check Exception
; 18: Machine Check Exceptio
; 19-31: Reserved
%assign i 15
%rep    17
isrstub_pseudo_error i
%assign i i+1
%endrep

; NASM macro for asynchronous interrupts (no exceptions)
%macro irqstub 1
global irq%1
irq%1:
push byte 0 ; pseudo error code
push byte 32+%1
jmp common_stub
%endmacro

; Create entries for the interrupts 0 to 23
%assign i 0
%rep    24
irqstub i
%assign i i+1
%endrep
