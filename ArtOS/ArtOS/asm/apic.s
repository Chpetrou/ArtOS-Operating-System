global apic_timer
apic_timer:
push byte 0 ; pseudo error code
push byte 123
jmp common_stub

global apic_lint0
apic_lint0:
push byte 0 ; pseudo error code
push byte 124
jmp common_stub

global apic_lint1
apic_lint1:
push byte 0 ; pseudo error code
push byte 125
jmp common_stub

global apic_error
apic_error:
push byte 0 ; pseudo error code
push byte 126
jmp common_stub

global apic_svr
apic_svr:
push byte 0 ; pseudo error code
push byte 127
jmp common_stub
